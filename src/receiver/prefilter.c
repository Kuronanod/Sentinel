#ifdef _WIN32
    #include <windows.h>
#else
    #include <pthread.h>
#endif

#include <string.h>
#include <time.h>

#include "prefilter.h"
#include "packet_counter.h"
#include "notification_queue.h"
#include "log_queue.h"
#include "firewall.h"
#include "receiver/log_queue.h"
#include "receiver/notification_queue.h"

#ifdef _WIN32
    static CRITICAL_SECTION BlacklistMutex;
    static CRITICAL_SECTION PortMutex;   // Windows
    static int PortMutexInitialized = 0;

    static int MutexInitialized = 0;
    static void LockBlacklist(void) {
        if (!MutexInitialized) {
            InitializeCriticalSection(&BlacklistMutex);
            MutexInitialized = 1;
        }
        EnterCriticalSection(&BlacklistMutex);
    }
    static void UnlockBlacklist(void) {
        LeaveCriticalSection(&BlacklistMutex);
    }

    static void LockPort(void) {
        if (!PortMutexInitialized) {
            InitializeCriticalSection(&PortMutex);
            PortMutexInitialized = 1;
        }
        EnterCriticalSection(&PortMutex);
    }

    static void UnlockPort(void) {
        LeaveCriticalSection(&PortMutex);
    }
    

#else
    #include <pthread.h>
    static pthread_mutex_t BlacklistMutex = PTHREAD_MUTEX_INITIALIZER;
    static void LockBlacklist(void) { pthread_mutex_lock(&BlacklistMutex); }
    static void UnlockBlacklist(void) { pthread_mutex_unlock(&BlacklistMutex); }
    static pthread_mutex_t PortMutex = PTHREAD_MUTEX_INITIALIZER;
    static void LockPort(void)   { pthread_mutex_lock(&PortMutex); }
    static void UnlockPort(void) { pthread_mutex_unlock(&PortMutex); }
#endif

#define MAX_BLACKLIST 100
#define MAX_TRACKED_IPS 256
#define DEFAULT_RATE 500
#define RATE_WINDOW_SEC 1
#define MAX_SUSPICIOUS_PORTS  50

static unsigned int Blacklist[MAX_BLACKLIST];
static int BlacklistCount = 0;
static int RateThreshold = DEFAULT_RATE;

static unsigned short SuspiciousPorts[MAX_SUSPICIOUS_PORTS] = {
    4444, 1337, 31337, 6667, 12345
};
static int SuspiciousPortsCount = 5;
static int SuspiciousPortsDefault = 5;

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

    int Added = 0;

    LockBlacklist(); 

    int Found = 0;
    for (int Index = 0; Index < BlacklistCount; Index++){
        if (Blacklist[Index] == IP){
            Found = 1;
            break;
        }
    }

    if (!Found && BlacklistCount < MAX_BLACKLIST) {
        Blacklist[BlacklistCount++] = IP;
        Added = 1;
    }

    UnlockBlacklist(); 

    if (Added) {
        FirewallBlockIP(IP);
        IncrementBlockedIPCount();
        LogWrite(LOG_INFO, "Blacklist added: %u.%u.%u.%u",
        IP & 0xFF,
        (IP >> 8) & 0xFF,
        (IP >> 16) & 0xFF,
        (IP >> 24) & 0xFF);
    }

}

void PreFilterRemoveBlacklist(unsigned int IP) {

    int Removed = 0;

    LockBlacklist();
    for (int Index = 0; Index < BlacklistCount; Index++) {
        if (Blacklist[Index] == IP) {
            Blacklist[Index] = Blacklist[BlacklistCount - 1];
            BlacklistCount--;
            Removed = 1;
            break;
        }
    }
    UnlockBlacklist();

    if (Removed) {
        FirewallUnblockIP(IP);
        DecrementBlockedIPCount();
        LogWrite(LOG_INFO, "Blacklist removed: %u.%u.%u.%u",
        IP & 0xFF,
        (IP >> 8) & 0xFF,
        (IP >> 16) & 0xFF,
        (IP >> 24) & 0xFF);
    }

}

int PreFilterGetBlacklistCount(void){

    LockBlacklist();
    int Count = BlacklistCount;
    UnlockBlacklist();
    return Count;

}

unsigned int PreFilterGetBlacklistIP(int Index) {
    LockBlacklist();
    unsigned int IP = 0;
    if (Index >= 0 && Index < BlacklistCount) {
        IP = Blacklist[Index];
    }
    UnlockBlacklist();
    return IP;
}

void PreFilterSetRateThreshold(int threshold) {
    if (threshold >= 10 && threshold <= 10000)
        RateThreshold = threshold;
}

int PreFilterGetRateThreshold(void){
    return RateThreshold; 
}

int PreFilterGetSuspiciousPortCount(void) {
    LockPort();
    int count = SuspiciousPortsCount;
    UnlockPort();
    return count;
}

unsigned short PreFilterGetSuspiciousPort(int index) {
    LockPort();
    unsigned short port = 0;
    if (index >= 0 && index < SuspiciousPortsCount) {
        port = SuspiciousPorts[index];
    }
    UnlockPort();
    return port;
}

// ================================================================
//  Add Port
// ================================================================
void PreFilterAddSuspiciousPort(unsigned short port) {
    int added = 0;

    LockPort();

    // ตรวจซ้ำ
    int found = 0;
    for (int i = 0; i < SuspiciousPortsCount; i++) {
        if (SuspiciousPorts[i] == port) {
            found = 1;
            break;
        }
    }

    if (!found && SuspiciousPortsCount < MAX_SUSPICIOUS_PORTS) {
        SuspiciousPorts[SuspiciousPortsCount++] = port;
        added = 1;
    }

    UnlockPort();

    if (added) {
        LogWrite(LOG_INFO, "Suspicious port added: %u", port);
        PushNotification(NOTIF_TYPE_SYSTEM,
            "Suspicious Port Added",
            "Port added to blocklist");
    }
}

// ================================================================
//  Remove Port
// ================================================================
void PreFilterRemoveSuspiciousPort(unsigned short port) {
    int removed = 0;

    LockPort();

    for (int i = 0; i < SuspiciousPortsCount; i++) {
        if (SuspiciousPorts[i] == port) {
            SuspiciousPorts[i] = SuspiciousPorts[SuspiciousPortsCount - 1];
            SuspiciousPortsCount--;
            removed = 1;
            break;
        }
    }

    UnlockPort();

    if (removed) {
        LogWrite(LOG_INFO, "Suspicious port removed: %u", port);
    }
}

// ================================================================
//  Reset to Default
// ================================================================
void PreFilterResetSuspiciousPorts(void) {
    LockPort();

    unsigned short defaults[] = {4444, 1337, 31337, 6667, 12345};
    SuspiciousPortsCount = SuspiciousPortsDefault;
    for (int i = 0; i < SuspiciousPortsDefault; i++) {
        SuspiciousPorts[i] = defaults[i];
    }

    UnlockPort();
}

void PreFilterClear(void) {

    LockBlacklist();

    BlacklistCount = 0;
    RateCount = 0;
    RateThreshold = DEFAULT_RATE;
    memset(RateTable, 0, sizeof(RateTable));

    UnlockBlacklist(); 

    FirewallClearAll();
    ResetBlockedIPCount();
    LogWrite(LOG_WARN, "All rules cleared"); 
    PushNotification(NOTIF_TYPE_SYSTEM, "Firewall Rule Added", 
    "IP added to blacklist");

}

static int IsBlacklisted(unsigned int IP) {

    int result = 0;

    LockBlacklist();

    for (int Index = 0; Index < BlacklistCount; Index++)
        if (Blacklist[Index] == IP){
            result = 1;
            break;
        }

    UnlockBlacklist(); 

    return result;
}

static int IsSuspiciousPort(unsigned short Port) {
    int result = 0;
    LockPort();
    for (int i = 0; i < SuspiciousPortsCount; i++) {
        if (SuspiciousPorts[i] == Port) {
            result = 1;
            break;
        }
    }
    UnlockPort();
    return result;
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