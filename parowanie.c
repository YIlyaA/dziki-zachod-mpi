#include "parowanie.h"

static void ready_list_add(int pid, int ts)
{
    pthread_mutex_lock(&ready_list_mut);

    int pos = ready_list_size;
    for (int i = 0; i < ready_list_size; i++) {
        if (ts < ready_list[i].ts || (ts == ready_list[i].ts && pid < ready_list[i].pid)) {
            pos = i;
            break;
        }
    }

    for (int i = ready_list_size; i > pos; i--)
        ready_list[i] = ready_list[i - 1];

    ready_list[pos].pid = pid;
    ready_list[pos].ts  = ts;
    ready_list_size++;

    pthread_mutex_unlock(&ready_list_mut);
}

static void ready_list_remove(int id1, int id2)
{
    pthread_mutex_lock(&ready_list_mut);
    int dst = 0;
    for (int i = 0; i < ready_list_size; i++) {
        if (ready_list[i].pid != id1 && ready_list[i].pid != id2) {
            ready_list[dst++] = ready_list[i];
        }
    }
    ready_list_size = dst;
    pthread_mutex_unlock(&ready_list_mut);
}

static void check_pairing()
{
    pthread_mutex_lock(&ready_list_mut);

    if (paired) {
        pthread_mutex_unlock(&ready_list_mut);
        return;
    }

    int pos = -1;
    for (int i = 0; i < ready_list_size; i++) {
        if (ready_list[i].pid == rank) {
            pos = i + 1; /* 1-based */
            break;
        }
    }

    if (pos < 0 || pos % 2 != 0) {
        pthread_mutex_unlock(&ready_list_mut);
        return;
    }

    int my_partner = ready_list[pos - 2].pid;
    paired = 1;
    partner_id = my_partner;

    int dst = 0;
    for (int i = 0; i < ready_list_size; i++) {
        if (ready_list[i].pid != rank && ready_list[i].pid != my_partner) {
            ready_list[dst++] = ready_list[i];
        }
    }
    ready_list_size = dst;

    pthread_mutex_unlock(&ready_list_mut);

    packet_t pkt;
    pkt.data = my_partner;
    for (int i = 0; i < size; i++)
        if (i != rank)
            sendPacket(&pkt, i, PAIR_CONFIRM);

    println("Sparowany z procesem %d", my_partner);
    changeState(IN_DUEL);
    pthread_mutex_lock(&state_cond_mut);
    pthread_cond_signal(&duel_cond);
    pthread_mutex_unlock(&state_cond_mut);
}

void send_ready()
{
    pthread_mutex_lock(&lamportMutex);
    lamportClock++;
    int ts = lamportClock;
    pthread_mutex_unlock(&lamportMutex);

    ready_list_add(rank, ts);

    packet_t pkt;
    pkt.ts = ts;
    pkt.src = rank;
    pkt.data = 0;
    for (int i = 0; i < size; i++)
        if (i != rank)
            MPI_Send(&pkt, 1, MPI_PAKIET_T, i, READY, MPI_COMM_WORLD);
}

void handle_ready(packet_t pkt)
{
    ready_list_add(pkt.src, pkt.ts);

    if (stan == WAIT_DUEL)
        check_pairing();
}

void handle_pair_confirm(packet_t pkt)
{
    int id1 = pkt.src;
    int id2 = pkt.data;

    if (rank == id1 || rank == id2) {
        partner_id = (rank == id1) ? id2 : id1;
        paired = 1;
        ready_list_remove(id1, id2);
        println("Sparowany z procesem %d", partner_id);
        changeState(IN_DUEL);
        pthread_mutex_lock(&state_cond_mut);
        pthread_cond_signal(&duel_cond);
        pthread_mutex_unlock(&state_cond_mut);
    } else {
        ready_list_remove(id1, id2);
        if (stan == WAIT_DUEL)
            check_pairing();
    }
}

void try_pairing()
{
    check_pairing();
}
