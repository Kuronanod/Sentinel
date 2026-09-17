#include <queue>
#include <string>
#include <mutex>
#include <cstring>

#include "alert_queue.h"

using namespace std;

static queue<string> GlobalAlertQueue;
static mutex GlobalAlertMutex;
static const int MAX_ALERT_QUEUE = 500;

void PushAlert(const char *Message) {

    lock_guard<mutex> lock(GlobalAlertMutex);
    if ((int)GlobalAlertQueue.size() >= MAX_ALERT_QUEUE){
        GlobalAlertQueue.pop();
    }
    GlobalAlertQueue.push(string(Message));

}

int PopAlert(char *Output, int MaxLen) {

    lock_guard<mutex> lock(GlobalAlertMutex);
    if (GlobalAlertQueue.empty()) return 0;
    string s = GlobalAlertQueue.front();
    GlobalAlertQueue.pop();
    strncpy(Output, s.c_str(), MaxLen - 1);
    Output[MaxLen - 1] = '\0';
    return 1;

}

int GetAlertQueueSize(void) {

    lock_guard<mutex> lock(GlobalAlertMutex);
    return (int)GlobalAlertQueue.size();

}

void ClearAlertQueue(void) {

    lock_guard<mutex> lock(GlobalAlertMutex);
    while (!GlobalAlertQueue.empty()) GlobalAlertQueue.pop();

}