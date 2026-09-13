#include <stdio.h>
#include <string.h>

#include "firewall.h"

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
#endif

static void FormatIP(unsigned int IP, char *Output, int MaxLen) {
    snprintf(Output, MaxLen, "%u.%u.%u.%u",
        IP & 0xFF,
        (IP >> 8) & 0xFF,
        (IP >> 16) & 0xFF,
        (IP >> 24) & 0xFF);
}

void FirewallBlockIP(unsigned int IP) {

#ifdef _WIN32

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

    RunHidden(Command);
    printf("[Firewall] Blocked: %s\n", IPString);

#endif

}

void FirewallUnblockIP(unsigned int IP) {

#ifdef _WIN32

    char IPString[32];
    FormatIP(IP, IPString, sizeof(IPString));

    char Command[512];
    snprintf(Command, sizeof(Command),
        "netsh advfirewall firewall delete rule "
        "name=\"Sentinel_Block_%s\"",
        IPString);

    RunHidden(Command);
    printf("[Firewall] Unblocked: %s\n", IPString);

#endif

}

void FirewallClearAll(void) {

#ifdef _WIN32
    
    RunHidden("powershell -Command \"Get-NetFirewallRule -DisplayName 'Sentinel_Block_*' -ErrorAction SilentlyContinue | Remove-NetFirewallRule\"");
    printf("[Firewall] Cleared all Sentinel rules\n");

#endif

}