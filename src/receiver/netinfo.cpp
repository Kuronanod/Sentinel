#include "netinfo.h"

#include <cstring>
#include <cstdio>

// ================================================================
//  Helper: ตรวจสอบ IP ที่ใช้งานได้
// ================================================================
static bool IsValidIP(const char *ip) {
    if (!ip || strlen(ip) == 0) return false;

    // ข้าม loopback
    if (strcmp(ip, "127.0.0.1") == 0) return false;

    // ข้าม APIPA (169.254.x.x) — IP ที่ Windows ใช้เมื่อหา DHCP ไม่เจอ
    if (strncmp(ip, "169.254.", 8) == 0) return false;

    // ข้าม 0.0.0.0
    if (strcmp(ip, "0.0.0.0") == 0) return false;

    return true;
}

// ================================================================
//  WINDOWS IMPLEMENTATION
// ================================================================
#ifdef _WIN32

#include <winsock2.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>

int GetLocalIP(char *out_ip, int bufSize) {
    if (!out_ip || bufSize < 16) return -1;

    // ---- Init Winsock ----
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return -1;
    }

    // ---- Get Adapters Info ----
    ULONG bufLen = 15000;
    PIP_ADAPTER_INFO pAdapterInfo = (IP_ADAPTER_INFO*)malloc(bufLen);

    if (!pAdapterInfo) {
        WSACleanup();
        return -1;
    }

    DWORD result = GetAdaptersInfo(pAdapterInfo, &bufLen);
    if (result != ERROR_SUCCESS) {
        free(pAdapterInfo);
        WSACleanup();
        return -1;
    }

    // ---- ค้นหา Interface ที่ใช้งานได้ ----
    PIP_ADAPTER_INFO pAdapter = pAdapterInfo;

    while (pAdapter) {
        // เฉพาะ Ethernet (6) และ Wi-Fi (71)
        if (pAdapter->Type == MIB_IF_TYPE_ETHERNET ||
            pAdapter->Type == IF_TYPE_IEEE80211) {

            IP_ADDR_STRING *ipAddr = &pAdapter->IpAddressList;

            while (ipAddr) {
                const char *ip = ipAddr->IpAddress.String;

                if (IsValidIP(ip)) {
                    strncpy(out_ip, ip, bufSize - 1);
                    out_ip[bufSize - 1] = '\0';

                    free(pAdapterInfo);
                    WSACleanup();
                    return 0;
                }
                ipAddr = ipAddr->Next;
            }
        }
        pAdapter = pAdapter->Next;
    }

    free(pAdapterInfo);
    WSACleanup();
    return -1;
}

int GetLocalIPByName(const char *ifName, char *out_ip, int bufSize) {
    // Windows ไม่ใช้ชื่อ interface แบบ Linux → เรียก GetLocalIP แทน
    (void)ifName;
    return GetLocalIP(out_ip, bufSize);
}

#endif // _WIN32

// ================================================================
//  LINUX IMPLEMENTATION
// ================================================================
#ifdef __linux__

#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int GetLocalIP(char *out_ip, int bufSize) {
    if (!out_ip || bufSize < 16) return -1;

    struct ifaddrs *ifaddr, *ifa;

    if (getifaddrs(&ifaddr) == -1) return -1;

    for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) continue;
        if (ifa->ifa_addr->sa_family != AF_INET) continue;

        // ข้าม loopback
        if (strcmp(ifa->ifa_name, "lo") == 0) continue;

        void *addr = &((struct sockaddr_in*)ifa->ifa_addr)->sin_addr;
        char ip[INET_ADDRSTRLEN];

        inet_ntop(AF_INET, addr, ip, INET_ADDRSTRLEN);

        if (IsValidIP(ip)) {
            strncpy(out_ip, ip, bufSize - 1);
            out_ip[bufSize - 1] = '\0';
            freeifaddrs(ifaddr);
            return 0;
        }
    }

    freeifaddrs(ifaddr);
    return -1;
}

int GetLocalIPByName(const char *ifName, char *out_ip, int bufSize) {
    if (!ifName || !out_ip || bufSize < 16) return -1;

    struct ifaddrs *ifaddr;

    if (getifaddrs(&ifaddr) == -1) return -1;

    for (struct ifaddrs *ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) continue;
        if (ifa->ifa_addr->sa_family != AF_INET) continue;
        if (strcmp(ifa->ifa_name, ifName) != 0) continue;

        void *addr = &((struct sockaddr_in*)ifa->ifa_addr)->sin_addr;
        inet_ntop(AF_INET, addr, out_ip, bufSize);

        freeifaddrs(ifaddr);
        return 0;
    }

    freeifaddrs(ifaddr);
    return -1;
}

#endif // __linux__