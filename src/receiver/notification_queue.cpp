#include "notification_queue.h"

#include <queue>
#include <mutex>
#include <cstring>
#include <ctime>
#include <atomic>

// ================================================================
//  Internal State
// ================================================================
static std::queue<NotificationData> g_notifQueue;
static std::mutex                   g_notifMutex;
static std::atomic<int>             g_unreadCount(0);
static const int                    MAX_QUEUE = 20;

// ================================================================
//  Push
// ================================================================
void PushNotification(int type, const char *title, const char *message) {
    NotificationData data;
    data.type = type;

    strncpy(data.title, title ? title : "", sizeof(data.title) - 1);
    data.title[sizeof(data.title) - 1] = '\0';

    strncpy(data.message, message ? message : "", sizeof(data.message) - 1);
    data.message[sizeof(data.message) - 1] = '\0';

    // ---- Time ----
    time_t now = time(nullptr);
    struct tm *tm_info = localtime(&now);
    strftime(data.time, sizeof(data.time), "%H:%M", tm_info);

    std::lock_guard<std::mutex> lock(g_notifMutex);

    // ---- Limit ----
    while ((int)g_notifQueue.size() >= MAX_QUEUE) {
        g_notifQueue.pop();
    }

    g_notifQueue.push(data);
    g_unreadCount++;
}

// ================================================================
//  Pop
// ================================================================
int PopNotification(NotificationData *out) {
    std::lock_guard<std::mutex> lock(g_notifMutex);
    if (g_notifQueue.empty()) return 0;

    *out = g_notifQueue.front();
    g_notifQueue.pop();
    return 1;
}

// ================================================================
//  Count
// ================================================================
int GetUnreadCount(void) {
    return g_unreadCount.load();
}

void MarkAllRead(void) {
    g_unreadCount = 0;
}

void ClearNotifications(void) {
    std::lock_guard<std::mutex> lock(g_notifMutex);
    while (!g_notifQueue.empty()) g_notifQueue.pop();
    g_unreadCount = 0;
}