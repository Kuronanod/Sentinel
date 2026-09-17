#ifndef READ_H
#define READ_H

#include "ring_buffer.h"

typedef struct{

    const char *data;
    int size;

}PacketData;

PacketData read_slot(RingBuffer *ring);

#endif