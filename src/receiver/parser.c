#include "receiver/parser.h"

#define PROTOCOL_TCP 6
#define PROTOCOL_UDP 17

PacketInfo parser(const char *buffer , int buffer_size){

    PacketInfo DATA;

    IPHeader *IP = (IPHeader *)buffer;

    DATA.SourceIP = IP->SourceAddress;
    DATA.DestinationIP = IP->DestinationAddress;
    DATA.Protocol = IP->Protocol;
    DATA.PacketLength = IP->TotalLength;

    if(IP->Protocol == PROTOCOL_UDP){

        UDPHeader *UDP = (UDPHeader *)(buffer + (IP->InternetHeaderLength * 4));

        DATA.SourcePort = UDP->SourcePort;
        DATA.DestinationPort = UDP->DestinationPort;

    }else if(IP->Protocol == PROTOCOL_TCP){

        TCPHeader *TCP = (TCPHeader *)(buffer + (IP->InternetHeaderLength * 4));

        DATA.SourcePort = TCP->SourcePort;
        DATA.DestinationPort = TCP->DestinationPort;
        DATA.TransmissionControlProtocol_Flags = TCP->Flags;

    }

    return DATA;

}