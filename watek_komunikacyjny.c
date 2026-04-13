#include "main.h"
#include "util.h"
#include "salon.h"
#include "parowanie.h"
#include "watek_komunikacyjny.h"

/* wątek komunikacyjny; zajmuje się odbiorem i reakcją na komunikaty */
void *startKomWatek(void *ptr)
{
    MPI_Status status;
    packet_t pakiet;
    while (1)
    {
        debug("czekam na recv");
        MPI_Recv(&pakiet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        pthread_mutex_lock(&lamportMutex);
        lamportClock = max(pakiet.ts, lamportClock) + 1;
        pthread_mutex_unlock(&lamportMutex);

        switch (status.MPI_TAG)
        {
        case REQ_SALON:
            debug("REQ_SALON od %d", status.MPI_SOURCE);
            handle_req_salon(pakiet);
            break;
        case ACK_SALON:
            debug("ACK_SALON od %d", status.MPI_SOURCE);
            handle_ack_salon(pakiet);
            break;
        case RELEASE_SALON:
            debug("RELEASE_SALON od %d (ignoruję)", status.MPI_SOURCE);
            break;
        case READY:
            debug("READY od %d ts=%d", status.MPI_SOURCE, pakiet.ts);
            handle_ready(pakiet);
            break;
        case PAIR_CONFIRM:
            debug("PAIR_CONFIRM od %d partner=%d", status.MPI_SOURCE, pakiet.data);
            handle_pair_confirm(pakiet);
            break;
        default:
            debug("Nieznany tag %d od %d", status.MPI_TAG, status.MPI_SOURCE);
            break;
        }
    }
    return NULL;
}
