#ifndef PACKET_COUNTER_H
#define PACKET_COUNTER_H

#ifdef __cplusplus
extern "C" {
#endif

int GetPacketCount(void);
void IncreementPacketCount(void);

int GetBlockedPacketCount(void);
void IncrementBlockedPacketCount(void);

#ifdef __cplusplus
}
#endif

#endif