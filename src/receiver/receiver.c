#include "receiver/windows/get_packet.h"
#include "receiver/windows/read.h"
#include "receiver/windows/socket.h"
#include "receiver/parser.h"
#include "receiver/packet_counter.h"
#include "receiver/request_queue.h"
#include "receiver/prefilter.h"
#include "receiver/alert_queue.h"

#include <stdio.h>
#include <stdbool.h>

int receiver(const char *ip){

    //printf("Receiver started on IP: %s\n", ip);
    SocketResult socket = init_socket(ip);

    if(socket.valid == false){
        printf("Socket init failed!\n");
        return -1;
    }

    //printf("Entering capture loop...\n");
    while(true){

        char buffer[65535];
        int size = get_packet(socket.socket , buffer , sizeof(buffer));
        
        if(size < 0){
            break;
        }

        PacketData DATA = read_packet(buffer , size);
        PacketInfo INFO = parser(DATA.data , DATA.size);

        PacketRecord Record;
        Record.SourceIP        = INFO.SourceIP;
        Record.DestinationIP   = INFO.DestinationIP;
        Record.SourcePort      = INFO.SourcePort;
        Record.DestinationPort = INFO.DestinationPort;
        Record.Protocol        = INFO.Protocol;
        Record.Flags           = INFO.TransmissionControlProtocol_Flags;
        Record.Length          = INFO.PacketLength;
        PushPacket(&Record);

        IncreementPacketCount();

        if (PreFilterCheck(&Record)) {

            IncrementBlockedPacketCount();
            char alertMsg[256];

            sprintf(alertMsg, "ALERT: Suspicious packet from %u.%u.%u.%u to port %u",
                Record.SourceIP & 0xFF,
                (Record.SourceIP >> 8) & 0xFF,
                (Record.SourceIP >> 16) & 0xFF,
                (Record.SourceIP >> 24) & 0xFF,
                Record.DestinationPort);
            PushAlert(alertMsg);
        } else {
            PushPacket(&Record);
        }

    }

    close_socket(&socket);
    return 0;

}