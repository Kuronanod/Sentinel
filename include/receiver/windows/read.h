#ifndef READ_H
#define READ_H

// Structure For PacketData
typedef struct{

    const char *data;
    int size;

}PacketData;

// Read Packet Function For Return Packet Buffer And Size To Parser
PacketData read_packet(const char *buffer , int size);

#endif