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

static volatile int GETBLOCKEDIPCOUNT = 0;

int  GetBlockedIPCount(void){
    return GETBLOCKEDIPCOUNT; 
}

void IncrementBlockedIPCount(void){
    GETBLOCKEDIPCOUNT++; 
}

void DecrementBlockedIPCount(void){
    if (GETBLOCKEDIPCOUNT > 0) {
        GETBLOCKEDIPCOUNT--;
    }
}

void ResetBlockedIPCount(void) {
    GETBLOCKEDIPCOUNT = 0;
}