#include <winsock2.h>
#include <windows.h>
#include <mstcpip.h>
#include <stdbool.h>
#include "include/receiver/windows/socket.h"

// Init Socket Function For Intialized Socket To Windows
SocketResult init_socket(const char *ip){

    SocketResult result;
    result.valid = false;

    WSADATA wsa;
    if(WSAStartup(MAKEWORD(2,2),&wsa) != 0){
        return result;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = 0;
    addr.sin_addr.s_addr = inet_addr(ip);

    DWORD option = 1;

    result.socket =  socket(AF_INET,SOCK_RAW,IPPROTO_IP);
    if(result.socket == INVALID_SOCKET){
        WSACleanup();
        return result;
    }

    if(bind(result.socket,(struct sockaddr *)&addr,sizeof(addr)) == SOCKET_ERROR){
        closesocket(result.socket);
        WSACleanup();
        return result;
    }

    if(ioctlsocket(result.socket,SIO_RCVALL,&option) == SOCKET_ERROR){
        closesocket(result.socket);
        WSACleanup();
        return result;
    }

    result.valid = true;

    return result;

}

// Close Socket Function For Close Socket To Windows
void close_socket(SocketResult *result){

    closesocket(result->socket);
    WSACleanup();

    result->valid = false;

}