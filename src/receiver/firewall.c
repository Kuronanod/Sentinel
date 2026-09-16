#include <stdio.h>
#include <string.h>

#include "firewall.h"
#include "log_queue.h"

#ifdef _WIN32
#include <windows.h>

static void RunHidden(const char *Command) {
    STARTUPINFOA StartInfomation = {0};
    PROCESS_INFORMATION ProcessInfomation = {0};
    StartInfomation.cb = sizeof(StartInfomation);
    StartInfomation.dwFlags = STARTF_USESHOWWINDOW;
    StartInfomation.wShowWindow = SW_HIDE;

    char CommandBuffer[512];
    strncpy(CommandBuffer, Command, sizeof(CommandBuffer) - 1);
    CommandBuffer[sizeof(CommandBuffer) - 1] = '\0';

    if (CreateProcessA(NULL, CommandBuffer, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &StartInfomation, &ProcessInfomation)) {
        WaitForSingleObject(ProcessInfomation.hProcess, 3000);
        CloseHandle(ProcessInfomation.hProcess);
        CloseHandle(ProcessInfomation.hThread);
    }
}

static void FormatIP(unsigned int IP, char *Output, int MaxLen) {
    snprintf(Output, MaxLen, "%u.%u.%u.%u",
        IP & 0xFF,
        (IP >> 8) & 0xFF,
        (IP >> 16) & 0xFF,
        (IP >> 24) & 0xFF);
}

void FirewallBlockIP(unsigned int IP) {

    char IPString[32];
    FormatIP(IP, IPString, sizeof(IPString));

    char Command[512];
    snprintf(Command, sizeof(Command),
        "netsh advfirewall firewall add rule "
        "name=\"Sentinel_Block_In_%s\" "
        "dir=in action=block remoteip=%s",
        IPString, IPString);
    RunHidden(Command);

    snprintf(Command, sizeof(Command),
        "netsh advfirewall firewall add rule "
        "name=\"Sentinel_Block_Out_%s\" "
        "dir=out action=block remoteip=%s",
        IPString, IPString);
    RunHidden(Command);

    printf("[Firewall] Blocked: %s\n", IPString);
    LogWrite(LOG_WARN, "Firewall block: %s", IPString);

}

void FirewallUnblockIP(unsigned int IP) {

    char IPString[32];
    FormatIP(IP, IPString, sizeof(IPString));

    char Command[512];
    snprintf(Command, sizeof(Command),
        "netsh advfirewall firewall delete rule name=\"Sentinel_Block_In_%s\"",
        IPString);
    RunHidden(Command);

    snprintf(Command, sizeof(Command),
        "netsh advfirewall firewall delete rule name=\"Sentinel_Block_Out_%s\"",
        IPString);
    RunHidden(Command);

    printf("[Firewall] Unblocked: %s\n", IPString);
    LogWrite(LOG_INFO, "Firewall unblock: %s", IPString);

}

void FirewallClearAll(void) {
    
    RunHidden("powershell -Command \"Get-NetFirewallRule -DisplayName 'Sentinel_Block_*' -ErrorAction SilentlyContinue | Remove-NetFirewallRule\"");
    printf("[Firewall] Cleared all Sentinel rules\n");

}

#else  // ========== Linux ==========

static void FormatIP(unsigned int IP, char *Output, int MaxLen) {
    snprintf(Output, MaxLen, "%u.%u.%u.%u",
        IP & 0xFF,
        (IP >> 8) & 0xFF,
        (IP >> 16) & 0xFF,
        (IP >> 24) & 0xFF);
}

static void RunShell(const char *Command) {
    system(Command);
}

void FirewallBlockIP(unsigned int IP) {
    char IPString[32];
    FormatIP(IP, IPString, sizeof(IPString));
    char Command[512];
    snprintf(Command, sizeof(Command),
        "sudo iptables -A INPUT -s %s -j DROP 2>/dev/null", IPString);
    RunShell(Command);
    snprintf(Command, sizeof(Command),
        "sudo iptables -A OUTPUT -d %s -j DROP 2>/dev/null", IPString);
    RunShell(Command);
}

void FirewallUnblockIP(unsigned int IP) {
    char IPString[32];
    FormatIP(IP, IPString, sizeof(IPString));
    char Command[512];
    snprintf(Command, sizeof(Command),
        "sudo iptables -D INPUT -s %s -j DROP 2>/dev/null", IPString);
    RunShell(Command);
    snprintf(Command, sizeof(Command),
        "sudo iptables -D OUTPUT -d %s -j DROP 2>/dev/null", IPString);
    RunShell(Command);
}

void FirewallClearAll(void) {
    // (ในเวอร์ชันจริงควรใช้ iptables-save | grep Sentinel)
    system("sudo iptables -F 2>/dev/null");
}

#endif