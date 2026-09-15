#ifndef PACKET_COUNTER_H
#define PACKET_COUNTER_H

#ifdef __cplusplus
extern "C" {
#endif

int GetPacketCount(void);
void IncreementPacketCount(void);

int GetBlockedPacketCount(void);
void IncrementBlockedPacketCount(void);

int  GetBlockedIPCount(void);
void IncrementBlockedIPCount(void);

void DecrementBlockedIPCount(void);
void ResetBlockedIPCount(void);

int  GetInboundCount(void);
void IncrementInboundCount(void);

int  GetOutboundCount(void);
void IncrementOutboundCount(void);


#ifdef __cplusplus
}
#endif

#endif