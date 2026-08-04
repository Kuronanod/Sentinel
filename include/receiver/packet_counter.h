#ifndef PACKET_COUNTER_H
#define PACKET_COUNTER_H

#ifdef __cplusplus
extern "C" {
#endif

int GetPacketCount(void);
void IncreementPacketCount(void);

#ifdef __cplusplus
}
#endif

#endif