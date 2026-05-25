#ifndef SOCKET_H
#define SOCKET_H

// Include Library For socket Function And Structure
#include <winsock2.h>
#include <winsock.h>
#include <stdbool.h>

// Structure For SOCKETRESULT
typedef struct{

    SOCKET socket;
    bool valid;

}SocketResult;

// SocketResult Function Export
SocketResult init_socket(const char *ip);

#endif