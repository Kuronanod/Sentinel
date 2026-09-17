#include "include/receiver/linux/read.h"
#include "include/receiver/linux/ring_buffer.h"
#include <linux/if_packet.h>
#include <stdint.h>

PacketData read_slot(RingBuffer *ring){
    
    struct tpacket_hdr *header = (struct tpacket_hdr *)(ring->Buffer + (ring->Current * 2048));

    if(header->tp_status == TP_STATUS_KERNEL){

        PacketData empty;
        empty.data = NULL;
        empty.size = 0;

        return empty;

    }else if(header->tp_status == TP_STATUS_USER){

        uint8_t *data = (uint8_t *)header + header->tp_mac;
        int size  = header->tp_len;
        header->tp_status = TP_STATUS_KERNEL;
        ring->Current = (ring->Current + 1) % ring->SlotCount;

        PacketData Packet;
        Packet.data = data;
        packet.size = size;
        return Packet;

    }

}