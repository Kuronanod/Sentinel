#ifndef SOCKET_H
#define SOCKET_H

// Include Library For socket Function And Structure
#include <winsock2.h>
#include <stdbool.h>

// Structure For SOCKETRESULT
typedef struct{

    SOCKET socket;
    bool valid;

}SocketResult;

// Intialized Socket Function Export
SocketResult init_socket(const char *ip);

// Close Socket Function Export
void close_socket(SocketResult *result);

#endif