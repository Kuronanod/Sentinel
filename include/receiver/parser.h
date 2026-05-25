#ifndef PARSER_H
#define PARSER_H

// Include Library For Parser Function And Structure
#include <stdint.h>
#include "windows/read.h"

// Structure For PACKETINFO
typedef struct{

    uint32_t SourceIP;
    uint32_t DestinationIP;
    uint16_t SourcePort;
    uint16_t DestinationPort;
    uint8_t Protocol;
    uint8_t TransmissionControlProtocol_Flags;
    uint32_t PacketLength;

}PacketInfo;

// Structure For IPHEADER
typedef struct{

    uint8_t Version:4 , InternetHeaderLength:4;
    uint8_t TypeOfService;
    uint16_t TotalLength;
    uint16_t Identification;
    uint16_t FragmentOffset;
    uint8_t TimeToLive;
    uint8_t Protocol;
    uint16_t HeaderChecksum;
    uint32_t SourceAddress;
    uint32_t DestinationAddress;

}IPHeader;

// Structure For UDPHeader
typedef struct{

    uint16_t SourcePort;
    uint16_t DestinationPort;
    uint16_t Length;
    uint16_t Checksum;

}UDPHeader;

// Structure For TCPHeader
typedef struct{

    uint16_t SourcePort;
    uint16_t DestinationPort;
    uint32_t SequenceNumber;
    uint32_t AcknowledgeNumber;
    uint8_t DataOffset;
    uint8_t Flags;
    uint16_t WindowSize;
    uint16_t Checksum;

}TCPHeader;

PacketInfo parser(const char *buffer , int buffer_size);

#endif