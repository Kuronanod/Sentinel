#include <string.h>
#include <time.h>

#include "prefilter.h"
#include "firewall.h"

#define MAX_BLACKLIST 100
#define MAX_TRACKED_IPS 256
#define DEFAULT_RATE 500
#define RATE_WINDOW_SEC 1

static unsigned int Blacklist[MAX_BLACKLIST];
static int BlacklistCount = 0;
static int RateThreshold = DEFAULT_RATE;


static const unsigned short SuspiciousPorts[] = {4444, 1337, 31337, 6667, 12345};
static const int SuspiciousPortsCount = 5;

typedef struct {

    unsigned int SourceIP;
    unsigned int DestinationIP;
    unsigned int IP;
    int Count;
    time_t LastReset;
    int Alerted;

} RateEntry;

static RateEntry RateTable[MAX_TRACKED_IPS];
static int RateCount = 0;

void PreFilterAddBlacklist(unsigned int IP) {

    for (int Index = 0; Index < BlacklistCount; Index++){
        if (Blacklist[Index] == IP){
            return;
        }
    }

    if (BlacklistCount < MAX_BLACKLIST) {
        Blacklist[BlacklistCount++] = IP;
        FirewallBlockIP(IP);
    }
}

void PreFilterRemoveBlacklist(unsigned int IP) {
    for (int Index = 0; Index < BlacklistCount; Index++) {
        if (Blacklist[Index] == IP) {
            Blacklist[Index] = Blacklist[BlacklistCount - 1];
            BlacklistCount--;
            FirewallUnblockIP(IP);
            return;
        }
    }
}

int PreFilterGetBlacklistCount(void) { return BlacklistCount; }

unsigned int PreFilterGetBlacklistIP(int Index) {
    if (Index >= 0 && Index < BlacklistCount){
        return Blacklist[Index];
    }
    return 0;

}

void PreFilterSetRateThreshold(int threshold) {
    if (threshold >= 10 && threshold <= 10000)
        RateThreshold = threshold;
}

int PreFilterGetRateThreshold(void){
    return RateThreshold; 
}

int PreFilterGetSuspiciousPortCount(void){
    return SuspiciousPortsCount; 
}

unsigned short PreFilterGetSuspiciousPort(int index) {
    if (index >= 0 && index < SuspiciousPortsCount){
        return SuspiciousPorts[index];
    }
    return 0;

}

void PreFilterClear(void) {
    BlacklistCount = 0;
    RateCount = 0;
    RateThreshold = DEFAULT_RATE;
    memset(RateTable, 0, sizeof(RateTable));
    FirewallClearAll();
}

static int IsBlacklisted(unsigned int IP) {
    for (int Index = 0; Index < BlacklistCount; Index++)
        if (Blacklist[Index] == IP){
            return 1;
        }
    return 0;
}

static int IsSuspiciousPort(unsigned short Port) {
    for (int Index = 0; Index < SuspiciousPortsCount; Index++)
        if (SuspiciousPorts[Index] == Port){
            return 1;
        }
    return 0;
}

static int CheckRate(unsigned int IP) {
    time_t TimeNow = time(NULL);

    for (int Index = 0; Index < RateCount; Index++) {
        if (RateTable[Index].IP == IP) {
            if (TimeNow - RateTable[Index].LastReset >= RATE_WINDOW_SEC) {
                RateTable[Index].Count = 0;
                RateTable[Index].LastReset = TimeNow;
                RateTable[Index].Alerted = 0;
                return 0;
            }

            RateTable[Index].Count++;

            if (RateTable[Index].Count > RateThreshold && !RateTable[Index].Alerted){
                return 1;
            }

            return 0;
        }
    }

    if (RateCount < MAX_TRACKED_IPS) {
        RateTable[RateCount].IP = IP;
        RateTable[RateCount].Count = 1;
        RateTable[RateCount].LastReset = TimeNow;
        RateTable[RateCount].Alerted = 0;
        RateCount++;
    }
    return 0;
}

int PreFilterCheck(const PacketRecord *Packet) {
    // 1. Blacklist
    if (IsBlacklisted(Packet->SourceIP) || IsBlacklisted(Packet->DestinationIP)) {
        return 1;
    }
    // 2. Suspicious Port
    if (IsSuspiciousPort(Packet->DestinationPort)) {
        return 1;
    }
    // 3. Rate Limit
    if (CheckRate(Packet->SourceIP)) {
        return 1;
    }
    return 0;
}