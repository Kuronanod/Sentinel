#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>

typedef struct{

    uint8_t *Buffer;
    size_t BufferSize;
    int SlotCount;
    int Current;

}RingBuffer;

RingBuffer setup_ring_buffer(int FileDescriptor);
void close_ring_buffer(RingBuffer *ring);

#endif