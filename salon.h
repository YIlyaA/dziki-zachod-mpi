#ifndef SALONH
#define SALONH
#include "main.h"

void salon_request();
void salon_release();
void handle_req_salon(packet_t pkt);
void handle_ack_salon(packet_t pkt);

#endif
