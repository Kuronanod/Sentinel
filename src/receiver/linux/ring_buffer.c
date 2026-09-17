#include "include/receiver/linux/ring_buffer.h"

RingBuffer setup_ring_buffer(int FileDescriptor){

    struct tpacket_req req;
    req.tp_block_size = 4096;
    req.tp_block_nr = 64;
    req.tp_frame_size = 2048;
    req.tp_frame_nr = 128;

    if(setsockopt(FileDescriptor , SOL_PACKET , PACKET_RX_RING,&req , sizeof(req)) == -1){
        RingBuffer ring;
        ring.Buffer = NULL;
        return ring;
    }

    size_t size = req.tp_block_size * req.tp_block_nr;

    void *buffer = mmap(NULL , size , PROT_READ|PROT_WRITE , MAP_SHARED , FileDescriptor , 0);

    if(buffer == MAP_FAILED){
        RingBuffer ring;
        ring.Buffer = NULL;
        return ring;
    }

    RingBuffer ring;
    ring.Buffer = buffer;
    ring.BufferSize = size;
    ring.SlotCount = req.tp_frame_nr;
    ring.Current = 0;

    return ring;

}

void close_ring_buffer(RingBuffer *ring){

    if(munmap(ring->Buffer,ring->BufferSize) == -1){
        return;
    }

    ring->Buffer = NULL;
    ring->BufferSize = 0;
    ring->SlotCount = 0;
    ring->Current = 0;

}