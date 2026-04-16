#include "salon.h"

void salon_request()
{
    ack_salon_count = 0;
    paired = 0;
    partner_id = -1;

    pthread_mutex_lock(&lamportMutex);
    lamportClock++;
    int ts = lamportClock;
    pthread_mutex_unlock(&lamportMutex);

    my_req_ts = ts;
    changeState(WAIT_SALON);

    packet_t pkt;
    pkt.ts = ts;
    pkt.src = rank;
    pkt.data = 0;
    for (int i = 0; i < size; i++) {
        if (i != rank) {
            MPI_Send(&pkt, 1, MPI_PAKIET_T, i, REQ_SALON, MPI_COMM_WORLD);
        }
    }
}

void salon_release()
{
    for (int i = 0; i < size; i++) {
        if (i != rank) {
            sendPacket(NULL, i, RELEASE_SALON);
        }
    }
    pthread_mutex_lock(&salon_queue_mut);
    for (int i = 0; i < salon_wait_queue_size; i++) {
        sendPacket(NULL, salon_wait_queue[i], ACK_SALON);
    }
    salon_wait_queue_size = 0;
    pthread_mutex_unlock(&salon_queue_mut);
}

void handle_req_salon(packet_t pkt)
{
    if (stan == WAIT_SALON) {
        if (pkt.ts < my_req_ts || (pkt.ts == my_req_ts && pkt.src < rank)) {
            sendPacket(NULL, pkt.src, ACK_SALON);
        } else {
            pthread_mutex_lock(&salon_queue_mut);
            salon_wait_queue[salon_wait_queue_size++] = pkt.src;
            pthread_mutex_unlock(&salon_queue_mut);
        }
    } else if (stan == IN_SALON || stan == WAIT_DUEL) {
        pthread_mutex_lock(&salon_queue_mut);
        salon_wait_queue[salon_wait_queue_size++] = pkt.src;
        pthread_mutex_unlock(&salon_queue_mut);
    } else {
        sendPacket(NULL, pkt.src, ACK_SALON);
    }
}

void handle_ack_salon(packet_t pkt)
{
    (void)pkt;
    if (stan == WAIT_SALON) {
        ack_salon_count++;
        if (ack_salon_count >= size - S) {
            changeState(IN_SALON);
            pthread_mutex_lock(&state_cond_mut);
            pthread_cond_signal(&salon_cond);
            pthread_mutex_unlock(&state_cond_mut);
        }
    }
}
