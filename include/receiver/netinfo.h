#ifndef NETINFO_H
#define NETINFO_H

#ifdef __cplusplus
extern "C" {
#endif

// ================================================================
//  ดึง Local IP ของเครื่อง
// ================================================================
// return: 0 = สำเร็จ, -1 = ไม่เจอ
// out_ip: buffer ขนาด >= 16 bytes
int GetLocalIP(char *out_ip, int bufSize);

// ดึง IP ตามชื่อ interface (Linux: "eth0", "wlan0")
int GetLocalIPByName(const char *ifName, char *out_ip, int bufSize);

#ifdef __cplusplus
}
#endif

#endif