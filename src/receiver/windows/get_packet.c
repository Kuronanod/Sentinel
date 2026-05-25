#include <winsock2.h>
#include "include/receiver/windows/socket.h"

// Get Packet Function For Get Raw Packet From Winsock
int get_packet(SOCKET socket , char *buffer , int buffer_size){

    struct sockaddr src;
    int src_len = sizeof(src);

    int byte = recvfrom(socket,buffer,buffer_size,0,&src,&src_len);

    if(byte == SOCKET_ERROR){
        return -1;
    }

    return byte;

}