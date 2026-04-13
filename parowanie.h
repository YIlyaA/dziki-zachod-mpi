#ifndef PAROWANIETH
#define PAROWANIETH
#include "main.h"

void send_ready();
void handle_ready(packet_t pkt);
void handle_pair_confirm(packet_t pkt);
void check_pairing();


#endif
