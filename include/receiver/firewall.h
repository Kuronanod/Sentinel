#ifndef FIREWALL_H
#define FIREWALL_H

#ifdef __cplusplus

extern "C" {

#endif

void FirewallBlockIP(unsigned int IP);
void FirewallUnblockIP(unsigned int IP);
void FirewallClearAll(void);

#ifdef __cplusplus

}

#endif

#endif