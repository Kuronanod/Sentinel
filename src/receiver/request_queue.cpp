#include <mutex>
#include <queue>

#include "receiver/request_queue.h"

using namespace std;

static queue<PacketRecord> GlobalPacketQueue;
static mutex GlobalMutexQueue;
static const int MAX_QUEUE_SIZE = 1000;

void PushPacket(const PacketRecord *Packet) {

    lock_guard<mutex> lock(GlobalMutexQueue);
    if ((int)GlobalPacketQueue.size() >= MAX_QUEUE_SIZE) {
        GlobalPacketQueue.pop();
    }
    GlobalPacketQueue.push(*Packet);

}

int PopPacket(PacketRecord *Output) {
    lock_guard<mutex> lock(GlobalMutexQueue);
    if (GlobalPacketQueue.empty()) return 0;
    *Output = GlobalPacketQueue.front();
    GlobalPacketQueue.pop();
    return 1;
}

void ClearPacketQueue(void) {
    lock_guard<mutex> lock(GlobalMutexQueue);
    while (!GlobalPacketQueue.empty()){
        GlobalPacketQueue.pop();
    }
}

int GetPacketQueueSize(void) {
    lock_guard<mutex> lock(GlobalMutexQueue);
    return (int)GlobalPacketQueue.size();
}