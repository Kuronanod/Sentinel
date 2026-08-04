#ifndef READ_H
#define READ_H

#ifdef __cplusplus
extern "C" {
#endif

// Structure For PacketData
typedef struct{

    const char *data;
    int size;

}PacketData;

// Read Packet Function For Return Packet Buffer And Size To Parser
PacketData read_packet(const char *buffer , int size);

#ifdef __cplusplus
}
#endif

#endif