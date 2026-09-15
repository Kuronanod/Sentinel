#include "receiver/parser.h"
#include "receiver/packet_counter.h"
#include "receiver/request_queue.h"
#include "receiver/prefilter.h"
#include "receiver/alert_queue.h"

#include <stdio.h>
#include <stdbool.h>

#ifdef _WIN32
    #include "windows/socket.h"
    #include "windows/get_packet.h"
    #include "windows/read.h"
#else
    #include "linux/socket.h"
    #include "linux/ring_buffer.h"
    #include "linux/read.h"
#endif

int receiver(const char *ip){

    #ifdef _WIN32
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

        IncreementPacketCount();

        if (Record.DestinationPort < 1024) {
            IncrementOutboundCount();
        } else {
            IncrementInboundCount();
        }

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

    #else

    // Linux: ใช้ interface name
    int FileDescriptor = open_socket(ip);   // ip = "eth0"
    if (FileDescriptor < 0) return -1;

    RingBuffer RING = setup_ring_buffer(FileDescriptor);
    if (!RING.Buffer) {
        close(FileDescriptor);
        return -1;
    }

    while (true) {
        PacketData DATA = read_slot(&RING);

        if (DATA.data == NULL){
            continue;
        }

        PacketInfo INFO = parser(DATA.data, DATA.size);

        PacketRecord Record;
        Record.SourceIP        = INFO.SourceIP;
        Record.DestinationIP   = INFO.DestinationIP;
        Record.SourcePort      = INFO.SourcePort;
        Record.DestinationPort = INFO.DestinationPort;
        Record.Protocol        = INFO.Protocol;
        Record.Flags           = INFO.TransmissionControlProtocol_Flags;
        Record.Length          = INFO.PacketLength;

        IncreementPacketCount();

        if (Record.DestinationPort < 1024) {
            IncrementOutboundCount();
        } else {
            IncrementInboundCount();
        }

        if (PreFilterCheck(&Record)) {

            IncrementBlockedPacketCount();
            char Message[256];

            sprintf(Message, "ALERT: Suspicious packet from %u.%u.%u.%u to port %u",
                Record.SourceIP & 0xFF,
                (Record.SourceIP >> 8) & 0xFF,
                (Record.SourceIP >> 16) & 0xFF,
                (Record.SourceIP >> 24) & 0xFF,
                Record.DestinationPort);
            PushAlert(Message);
        } else {
            PushPacket(&Record);
        }

    }

    close_ring_buffer(&RING);
    close(FileDescriptor);
    #endif

    return 0;

}