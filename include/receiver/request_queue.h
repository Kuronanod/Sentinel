#ifndef REQUEST_QUEUE_H
#define REQUEST_QUEUE_H

#ifdef __cplusplus
extern "C" {

#endif

typedef struct {
    unsigned int   SourceIP;
    unsigned int   DestinationIP;
    unsigned short SourcePort;
    unsigned short DestinationPort;
    unsigned char  Protocol;
    unsigned char  Flags;
    unsigned int   Length;
} PacketRecord;

void PushPacket(const PacketRecord *Packet);

int PopPacket(PacketRecord *Output);

void ClearPacketQueue(void);

int GetPacketQueueSize(void);

#ifdef __cplusplus

}
#endif

#endif