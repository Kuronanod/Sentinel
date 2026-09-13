#ifndef PREFILTER_H
#define PREFILTER_H

#include "request_queue.h"

#ifdef __cplusplus

extern "C" {

#endif

int PreFilterCheck(const PacketRecord *Packet);
void PreFilterClear(void);

void PreFilterAddBlacklist(unsigned int IP);
void PreFilterRemoveBlacklist(unsigned int IP);
int  PreFilterGetBlacklistCount(void);
unsigned int PreFilterGetBlacklistIP(int Index);

void PreFilterSetRateThreshold(int Threshold);
int  PreFilterGetRateThreshold(void);

int  PreFilterGetSuspiciousPortCount(void);
unsigned short PreFilterGetSuspiciousPort(int Index);

#ifdef __cplusplus

}

#endif

#endif