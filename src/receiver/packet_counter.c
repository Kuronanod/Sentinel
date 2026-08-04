#include "receiver/packet_counter.h"

static volatile int GETPACKETCOUNT = 0;

int GetPacketCount(void){

    return GETPACKETCOUNT;

}

void IncreementPacketCount(void){

    GETPACKETCOUNT++;

}