#include "receiver/packet_counter.h"

static volatile int GETPACKETCOUNT = 0;

int GetPacketCount(void){

    return GETPACKETCOUNT;

}

void IncreementPacketCount(void){

    GETPACKETCOUNT++;

}

static volatile int GETBLOCKEDPACKETCOUNT = 0;

int GetBlockedPacketCount(void) {
    return GETBLOCKEDPACKETCOUNT;
}

void IncrementBlockedPacketCount(void) {
    GETBLOCKEDPACKETCOUNT++;
}