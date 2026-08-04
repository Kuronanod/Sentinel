#ifndef GET_PACKET_H
#define GET_PACKET_H

// Include Library For get_packet Function
#include <winsock2.h>

#ifdef __cplusplus
extern "C" {
#endif

// Get Packet Function For Get Raw Packet From Winsock
int get_packet(SOCKET socket , char *buffer , int buffer_size);

#ifdef __cplusplus
}
#endif

#endif