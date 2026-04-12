#include "salon.h"

void salon_request()
{
    ack_salon_count = 0;
    paired = 0;
    partner_id = -1;
    for (int i = 0; i < size; i++){
        if(i != rank){
            sendPacket(NULL, i, REQ_SALON);
        }
    }
    my_req_ts = lamportClock;
    changeState(WAIT_SALON);
}

void salon_release()
{
    for(int i = 0; i < size; i++){
        if(i != rank){
            sendPacket(NULL, i, RELEASE_SALON);
        }
    }
}

void handle_req_salon(packet_t pkt)
{
    (void)pkt;
    if(stan == IN_SALON){
        if(pkt.ts < my_req_ts || (pkt.ts == my_req_ts && pkt.src < rank)){
            sendPacket(NULL, pkt.src, ACK_SALON);
        }
        else{
            salon_wait_queue[ack_salon_count++] = pkt.src;
        }
    }
    else{
        sendPacket(NULL, pkt.src, ACK_SALON);
    }
}

void handle_ack_salon(packet_t pkt)
{
    (void)pkt;
    if(stan == WAIT_SALON){
        ack_salon_count++;
        if(ack_salon_count >= size - S){
            changeState(IN_SALON);
            pthread_mutex_lock(&state_cond_mut);
            pthread_cond_signal(&salon_cond);
            pthread_mutex_unlock(&state_cond_mut);
        }
    }
}
