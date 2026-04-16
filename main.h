#ifndef MAINH
#define MAINH
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <pthread.h>

#include "util.h"
#define SEC_IN_STATE 3

#define max(a, b) ((a) > (b) ? (a) : (b))

/* tutaj TYLKO zapowiedzi - definicje w main.c */
extern int rank;
extern int size;
extern pthread_t threadKom;

extern int lamportClock;
extern pthread_mutex_t lamportMutex;

/* ---- Parametr salonu ---- */
#define MAX_PROCS 64
extern int S;

/* ---- Algorytm wejścia do salonu (salon.c) ---- */
extern int my_req_ts;
extern int ack_salon_count;

extern int salon_wait_queue[MAX_PROCS];
extern int salon_wait_queue_size;
extern pthread_mutex_t salon_queue_mut;

/* ---- Algorytm doboru w pary (parowanie.c) ---- */
typedef struct
{
    int pid;
    int ts;
} ready_entry_t;
extern ready_entry_t ready_list[MAX_PROCS];
extern int ready_list_size;
extern pthread_mutex_t ready_list_mut;

extern int partner_id;
extern int partner_ts;
extern int paired;

extern int my_ready_ts;
extern int ack_ready_count;

/* ---- Condition variables dla głównego wątku ---- */
extern pthread_cond_t salon_cond;
extern pthread_cond_t duel_cond;
extern pthread_mutex_t state_cond_mut;

/* macro debug - działa jak printf, kiedy zdefiniowano
   DEBUG, kiedy DEBUG niezdefiniowane działa jak instrukcja pusta

   używa się dokładnie jak printfa, tyle, że dodaje kolorków i automatycznie
   wyświetla rank

   w związku z tym, zmienna "rank" musi istnieć.

   w printfie: definicja znaku specjalnego "%c[%d;%dm [%d]" escape[styl bold/normal;kolor [RANK]
                                           FORMAT:argumenty doklejone z wywołania debug poprzez __VA_ARGS__
                       "%c[%d;%dm"       wyczyszczenie atrybutów    27,0,37
                                            UWAGA:
                                                27 == kod ascii escape.
                                                Pierwsze %c[%d;%dm ( np 27[1;10m ) definiuje styl i kolor literek
                                                Drugie   %c[%d;%dm czyli 27[0;37m przywraca domyślne kolory i brak pogrubienia (bolda)
                                                ...  w definicji makra oznacza, że ma zmienną liczbę parametrów

*/
#ifdef DEBUG
#define debug(FORMAT, ...)                                                                                                                                        \
    {                                                                                                                                                             \
        pthread_mutex_lock(&lamportMutex);                                                                                                                        \
        printf("%c[%d;%dm [%d] (Zegar: %d): " FORMAT "%c[%d;%dm\n", 27, (1 + (rank / 7)) % 2, 31 + (6 + rank) % 7, rank, lamportClock, ##__VA_ARGS__, 27, 0, 37); \
        pthread_mutex_unlock(&lamportMutex);                                                                                                                      \
    }
#else
#define debug(...) ;
#endif

// makro println - to samo co debug, ale wyświetla się zawsze
#define println(FORMAT, ...)                                                                                                                                      \
    {                                                                                                                                                             \
        pthread_mutex_lock(&lamportMutex);                                                                                                                        \
        printf("%c[%d;%dm [%d] (Zegar: %d): " FORMAT "%c[%d;%dm\n", 27, (1 + (rank / 7)) % 2, 31 + (6 + rank) % 7, rank, lamportClock, ##__VA_ARGS__, 27, 0, 37); \
        pthread_mutex_unlock(&lamportMutex);                                                                                                                      \
    }

#endif
