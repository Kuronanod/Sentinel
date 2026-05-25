#include "include/receiver/windows/read.h"

// Read Packet Function For Return Packet Buffer And Size To Parser
PacketData read_packet(const char *buffer , int size){

    PacketData packet;
    packet.size = size;
    packet.data = buffer;

    return packet;

}
