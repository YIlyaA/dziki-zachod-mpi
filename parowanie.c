#include "parowanie.h"

/* Wstawia proces do ready_list zachowując sortowanie po (ts, pid).
 * Wywołuj tylko z wątku komunikacyjnego lub gdy masz ready_list_mut. */
static void ready_list_add(int pid, int ts)
{
    pthread_mutex_lock(&ready_list_mut);

    /* znajdź pozycję wstawienia */
    int pos = ready_list_size;
    for (int i = 0; i < ready_list_size; i++) {
        if (ts < ready_list[i].ts || (ts == ready_list[i].ts && pid < ready_list[i].pid)) {
            pos = i;
            break;
        }
    }

    /* przesuń elementy w prawo */
    for (int i = ready_list_size; i > pos; i--)
        ready_list[i] = ready_list[i - 1];

    ready_list[pos].pid = pid;
    ready_list[pos].ts  = ts;
    ready_list_size++;

    pthread_mutex_unlock(&ready_list_mut);
}

/* Usuwa parę: parzysty element (pid_even) i jego poprzednik (nieparzysty).
 * Listy są identyczne u wszystkich procesów (ten sam sort po Lamporcie),
 * więc poprzednik parzystego to zawsze jego partner. Usuwamy atomowo. */
static void ready_list_remove_pair(int pid_even)
{
    pthread_mutex_lock(&ready_list_mut);

    for (int i = 1; i < ready_list_size; i++) {
        if (ready_list[i].pid == pid_even) {
            /* i-1 — nieparzysty partner, i — parzysty inicjator */
            for (int j = i - 1; j < ready_list_size - 2; j++)
                ready_list[j] = ready_list[j + 2];
            ready_list_size -= 2;
            break;
        }
    }

    pthread_mutex_unlock(&ready_list_mut);
}

static void check_pairing()
{
    /* znajdź swoją pozycję w ready_list (1-based) */
    pthread_mutex_lock(&ready_list_mut);
    int pos = -1;
    for (int i = 0; i < ready_list_size; i++) {
        if (ready_list[i].pid == rank) {
            pos = i + 1; /* 1-based */
            break;
        }
    }
    int my_partner = (pos >= 2) ? ready_list[pos - 2].pid : -1;
    pthread_mutex_unlock(&ready_list_mut);

    /* nie ma nas na liście lub jesteśmy na nieparzystej pozycji — czekamy */
    if (pos < 0 || pos % 2 != 0)
        return;

    /* jesteśmy na parzystej pozycji — ogłaszamy parę wszystkim */
    packet_t pkt;
    pkt.data = my_partner;
    for (int i = 0; i < size; i++)
        if (i != rank)
            sendPacket(&pkt, i, PAIR_CONFIRM);

    /* sami też obsługujemy PAIR_CONFIRM — logika przejścia jest w jednym miejscu */
    handle_pair_confirm(pkt);
}

void send_ready()
{
    println("Ogłaszam gotowość do pojedynku");

    /* najpierw dodaj siebie — żeby być w liście przed wysłaniem READY */
    ready_list_add(rank, lamportClock);

    for (int i = 0; i < size; i++)
        if (i != rank)
            sendPacket(NULL, i, READY);
}

void handle_ready(packet_t pkt)
{
    ready_list_add(pkt.src, pkt.ts);

    if (stan == WAIT_DUEL)
        check_pairing();
}

void handle_pair_confirm(packet_t pkt)
{
    int id_even = pkt.src;  /* parzysty — inicjator */
    int id_odd  = pkt.data; /* nieparzysty — partner */

    /* każdy usuwa tę parę ze swojego ready_list */
    ready_list_remove_pair(id_even);

    if (rank == id_even || rank == id_odd) {
        /* jesteśmy jednym z uczestników pary */
        partner_id = (rank == id_even) ? id_odd : id_even;
        paired = 1;
        println("Sparowany z procesem %d", partner_id);
        changeState(IN_DUEL);
        pthread_mutex_lock(&state_cond_mut);
        pthread_cond_signal(&duel_cond);
        pthread_mutex_unlock(&state_cond_mut);
    } else {
        /* jesteśmy obserwatorem — pozycja mogła się zmienić, sprawdzamy */
        if (stan == WAIT_DUEL)
            check_pairing();
    }
}
