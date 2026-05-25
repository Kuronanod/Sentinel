#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <net/if.h>

int open_socket(const char *interface){

    int FileDescriptor = socket(AF_PACKET,SOCK_RAW,htons(ETH_P_ALL));    

    if(FileDescriptor < 0){
        return - 1;
    }

    int InterfaceIndex = if_nametoindex(interface);
    if(InterfaceIndex == 0){
        return -1;
    }

    struct sockaddr_ll addr;
    addr.sll_family = AF_PACKET;
    addr.sll_protocol = htons(ETH_P_ALL);
    addr.sll_ifindex = InterfaceIndex;

    if(bind(FileDescriptor,&addr,sizeof(addr)) < 0){
        close(FileDescriptor);
        return -1;
    }

    return FileDescriptor;

}