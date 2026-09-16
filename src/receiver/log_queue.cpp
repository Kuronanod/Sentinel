#include "log_queue.h"

#include <queue>
#include <string>
#include <mutex>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <ctime>

// ================================================================
//  Internal State
// ================================================================
struct LogEntry {
    std::string time;
    int         level;
    std::string message;
};

static std::queue<LogEntry> g_logQueue;
static std::mutex           g_logMutex;
static const int            MAX_QUEUE = 2000;

static FILE* g_logFile = nullptr;
static char  g_logFilePath[512] = {0};
static int   g_fileEnabled = 0;

// ================================================================
//  Init / Close
// ================================================================
void LogInit(const char *filePath) {
    std::lock_guard<std::mutex> lock(g_logMutex);

    if (g_logFile) {
        fclose(g_logFile);
        g_logFile = nullptr;
    }

    if (filePath && strlen(filePath) > 0) {
        g_logFile = fopen(filePath, "a");
        if (g_logFile) {
            strncpy(g_logFilePath, filePath, sizeof(g_logFilePath) - 1);
            g_fileEnabled = 1;
        } else {
            g_fileEnabled = 0;
            g_logFilePath[0] = '\0';
        }
    } else {
        g_fileEnabled = 0;
        g_logFilePath[0] = '\0';
    }
}

void LogClose(void) {
    std::lock_guard<std::mutex> lock(g_logMutex);
    if (g_logFile) {
        fclose(g_logFile);
        g_logFile = nullptr;
    }
}

// ================================================================
//  Write
// ================================================================
void LogWrite(int level, const char *fmt, ...) {
    // ---- Format message ----
    char msg[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, sizeof(msg), fmt, args);
    va_end(args);

    // ---- Time ----
    time_t now = time(nullptr);
    struct tm *tm_info = localtime(&now);
    char timeStr[32];
    strftime(timeStr, sizeof(timeStr), "%H:%M:%S", tm_info);

    // ---- Create entry ----
    LogEntry entry;
    entry.time    = timeStr;
    entry.level   = level;
    entry.message = msg;

    // ---- Push to queue ----
    {
        std::lock_guard<std::mutex> lock(g_logMutex);

        if ((int)g_logQueue.size() >= MAX_QUEUE) {
            g_logQueue.pop();
        }
        g_logQueue.push(entry);

        // ---- Write to file ----
        if (g_fileEnabled && g_logFile) {
            const char *levelStr = "DEBUG";
            if      (level == LOG_INFO) levelStr = "INFO";
            else if (level == LOG_WARN) levelStr = "WARN";
            else if (level == LOG_CRIT) levelStr = "CRIT";

            fprintf(g_logFile, "[%s] [%s] %s\n",
                    timeStr, levelStr, msg);
            fflush(g_logFile);
        }
    }
}

// ================================================================
//  Pop for UI
// ================================================================
int LogPop(char *out, int maxLen, int *outLevel, char *outTime) {
    std::lock_guard<std::mutex> lock(g_logMutex);

    if (g_logQueue.empty()) return 0;

    LogEntry entry = g_logQueue.front();
    g_logQueue.pop();

    strncpy(out, entry.message.c_str(), maxLen - 1);
    out[maxLen - 1] = '\0';

    if (outLevel) *outLevel = entry.level;
    if (outTime) {
        strncpy(outTime, entry.time.c_str(), 31);
        outTime[31] = '\0';
    }
    return 1;
}

int LogGetQueueSize(void) {
    std::lock_guard<std::mutex> lock(g_logMutex);
    return (int)g_logQueue.size();
}

void LogClearQueue(void) {
    std::lock_guard<std::mutex> lock(g_logMutex);
    while (!g_logQueue.empty()) g_logQueue.pop();
}

// ================================================================
//  File Control
// ================================================================
void LogSetFileEnabled(int enabled) {
    std::lock_guard<std::mutex> lock(g_logMutex);
    g_fileEnabled = enabled ? 1 : 0;

    if (enabled && !g_logFile && g_logFilePath[0]) {
        g_logFile = fopen(g_logFilePath, "a");
    }
    if (!enabled && g_logFile) {
        fclose(g_logFile);
        g_logFile = nullptr;
    }
}

int LogIsFileEnabled(void) {
    std::lock_guard<std::mutex> lock(g_logMutex);
    return g_fileEnabled;
}

const char* LogGetFilePath(void) {
    return g_logFilePath;
}