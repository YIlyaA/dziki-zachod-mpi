/* w main.h także makra println oraz debug -  z kolorkami! */
#include "main.h"
#include "watek_glowny.h"
#include "watek_komunikacyjny.h"

/*
 * W main.h extern int rank (zapowiedź) w main.c int rank (definicja)
 * Zwróćcie uwagę, że każdy proces ma osobą pamięć, ale w ramach jednego
 * procesu wątki współdzielą zmienne - więc dostęp do nich powinien
 * być obwarowany muteksami. Rank i size akurat są write-once, więc nie trzeba,
 * ale zob util.c oraz util.h - zmienną state_t state i funkcję changeState
 *
 */
int rank, size;
pthread_t threadKom;

int lamportClock = 0;
pthread_mutex_t lamportMutex = PTHREAD_MUTEX_INITIALIZER;

int S = 2;

int my_req_ts = 0;
int ack_salon_count = 0;

int salon_wait_queue[MAX_PROCS];
int salon_wait_queue_size = 0;
pthread_mutex_t salon_queue_mut = PTHREAD_MUTEX_INITIALIZER;

ready_entry_t ready_list[MAX_PROCS];
int ready_list_size = 0;
pthread_mutex_t ready_list_mut = PTHREAD_MUTEX_INITIALIZER;

int partner_id = -1;
int paired = 0;

pthread_cond_t salon_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t duel_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t state_cond_mut = PTHREAD_MUTEX_INITIALIZER;

void finalizuj()
{
    pthread_mutex_destroy(&stateMut);
    pthread_mutex_destroy(&salon_queue_mut);
    pthread_mutex_destroy(&ready_list_mut);
    pthread_mutex_destroy(&state_cond_mut);
    pthread_cond_destroy(&salon_cond);
    pthread_cond_destroy(&duel_cond);
    println("czekam na wątek \"komunikacyjny\"");
    pthread_join(threadKom, NULL);
    MPI_Type_free(&MPI_PAKIET_T);
    MPI_Finalize();
}

void check_thread_support(int provided)
{
    printf("THREAD SUPPORT: chcemy %d. Co otrzymamy?\n", provided);
    switch (provided)
    {
    case MPI_THREAD_SINGLE:
        printf("Brak wsparcia dla wątków, kończę\n");
        /* Nie ma co, trzeba wychodzić */
        fprintf(stderr, "Brak wystarczającego wsparcia dla wątków - wychodzę!\n");
        MPI_Finalize();
        exit(-1);
        break;
    case MPI_THREAD_FUNNELED:
        printf("tylko te wątki, ktore wykonaly mpi_init_thread mogą wykonać wołania do biblioteki mpi\n");
        break;
    case MPI_THREAD_SERIALIZED:
        /* Potrzebne zamki wokół wywołań biblioteki MPI */
        printf("tylko jeden watek naraz może wykonać wołania do biblioteki MPI\n");
        break;
    case MPI_THREAD_MULTIPLE:
        printf("Pełne wsparcie dla wątków\n"); /* tego chcemy. Wszystkie inne powodują problemy */
        break;
    default:
        printf("Nikt nic nie wie\n");
    }
}

int main(int argc, char **argv)
{
    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    check_thread_support(provided);
    /* zob. util.c oraz util.h */
    inicjuj_typ_pakietu(); // tworzy typ pakietu
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (argc >= 2)
        S = atoi(argv[1]);
    if (S < 2 || S >= size)
    {
        if (rank == 0)
            fprintf(stderr, "Nieprawidłowe S=%d dla n=%d (min 2 — potrzeba pary w salonie). S=2.\n", S, size);
        S = 2;
    }
    /* startKomWatek w watek_komunikacyjny.c
     * w vi najedź kursorem na nazwę pliku i wciśnij klawisze gf
     * powrót po wciśnięciu ctrl+6
     * */
    pthread_create(&threadKom, NULL, startKomWatek, 0);

    /* mainLoop w watek_glowny.c
     * w vi najedź kursorem na nazwę pliku i wciśnij klawisze gf
     * powrót po wciśnięciu ctrl+6
     * */
    mainLoop(); // możesz także wcisnąć ctrl-] na nazwie funkcji
                // działa, bo używamy ctags (zob Makefile)
                // jak nie działa, wpisz set tags=./tags :)

    finalizuj();
    return 0;
}
