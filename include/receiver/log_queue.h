#ifndef LOG_QUEUE_H
#define LOG_QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

// ================================================================
//  Log Levels
// ================================================================
#define LOG_DEBUG  0
#define LOG_INFO   1
#define LOG_WARN   2
#define LOG_CRIT   3

// ================================================================
//  API
// ================================================================
void LogInit(const char *filePath);   // เปิดไฟล์ log (path = NULL → ไม่เขียนไฟล์)
void LogClose(void);

void LogWrite(int level, const char *fmt, ...);   // printf-style

// ---- สำหรับ UI ----
// return: 1 = มีข้อมูล, 0 = ไม่มี
int  LogPop(char *out, int maxLen, int *outLevel, char *outTime);

int  LogGetQueueSize(void);
void LogClearQueue(void);

// ---- File control ----
void LogSetFileEnabled(int enabled);
int  LogIsFileEnabled(void);
const char* LogGetFilePath(void);

#ifdef __cplusplus
}
#endif

#endif