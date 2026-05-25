#include "include/receiver/windows/get_packet.h"
#include "include/receiver/windows/read.h"
#include "include/receiver/windows/socket.h"
#include "include/receiver/parser.h"
#include <stdbool.h>

int receiver(const char *ip){

    SocketResult socket = init_socket(ip);

    if(socket.valid == false){
        return -1;
    }

    while(true){

        char buffer[65535];
        int size = get_packet(socket.socket , buffer , sizeof(buffer));
        
        if(size < 0){
            break;
        }

        PacketData DATA = read_packet(buffer , size);
        PacketInfo INFO = parser(DATA.data , DATA.size);

    }

    close_socket(&socket);
    return 0;

}