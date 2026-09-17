#ifndef NOTIFICATION_QUEUE_H
#define NOTIFICATION_QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

// ================================================================
//  Notification Types
// ================================================================
#define NOTIF_TYPE_ALERT   0    // 🚨
#define NOTIF_TYPE_AI      1    // 🤖
#define NOTIF_TYPE_SYSTEM  2    // ✓

// ================================================================
//  Data Structure
// ================================================================
typedef struct {
    int   type;              // ALERT / AI / SYSTEM
    char  title[128];        // "Anomaly Detected"
    char  message[256];      // "Z-Score 5.5 — 890 pps"
    char  time[16];          // "14:32"
} NotificationData;

// ================================================================
//  API
// ================================================================
void PushNotification(int type, const char *title, const char *message);
int  PopNotification(NotificationData *out);   // 1 = has, 0 = empty
int  GetUnreadCount(void);
void MarkAllRead(void);
void ClearNotifications(void);

#ifdef __cplusplus
}
#endif

#endif