#ifndef GET_PACKET_H
#define GET_PACKET_H

// Include Library For get_packet Function
#include <winsock2.h>

// Get Packet Function For Get Raw Packet From Winsock
int get_packet(SOCKET socket , const char *buffer , int buffer_size);

#endif