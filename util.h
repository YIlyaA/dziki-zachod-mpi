#ifndef UTILH
#define UTILH
#include "main.h"

/* typ pakietu */
typedef struct
{
    int ts; /* timestamp (zegar lamporta */
    int src;

    int data; /* przykładowe pole z danymi; można zmienić nazwę na bardziej pasującą */
} packet_t;
/* packet_t ma trzy pola, więc NITEMS=3. Wykorzystane w inicjuj_typ_pakietu */
#define NITEMS 3

/* Typy wiadomości */
#define REQ_SALON 1
#define ACK_SALON 2
#define RELEASE_SALON 3
#define READY 4
#define PAIR_CONFIRM 5

extern MPI_Datatype MPI_PAKIET_T;
void inicjuj_typ_pakietu();

/* wysyłanie pakietu, skrót: wskaźnik do pakietu (0 oznacza stwórz pusty pakiet), do kogo, z jakim typem */
void sendPacket(packet_t *pkt, int destination, int tag);

typedef enum
{
    REST,       /* odpoczywa */
    WAIT_SALON, /* wysłał REQ_SALON, czeka na ACK_SALON */
    IN_SALON,   /* w salonie */
    WAIT_DUEL,  /* wysłał READY, czeka na parę */
    IN_DUEL,    /* w pojedynku */
    HOSPITAL    /* przegrał, leczy się */
} state_t;
extern state_t stan;
extern pthread_mutex_t stateMut;
/* zmiana stanu, obwarowana muteksem */
void changeState(state_t);

/* --- salon.c --- */
void salon_request();
void salon_release();
void handle_req_salon(packet_t pkt);
void handle_ack_salon(packet_t pkt);

/* --- parowanie.c --- */
void send_ready();
void handle_ready(packet_t pkt);
void handle_pair_confirm(packet_t pkt);
#endif
