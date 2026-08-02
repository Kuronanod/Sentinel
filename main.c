#include <stdio.h>

#include "include/receiver/receiver.h"
#include "include/curses/curses.h"

int main(int argc , char *argv[]){

    if(argc < 2){

        #ifdef _WIN32
            printf("USAGE: sentinel.exe <ip>\n");
            printf("EXAMPLE: sentinel.exe 192.168.1.100\n");
        #elif defined(__linux__)
            printf("USAGE: ./sentinel <interface>\n");
            printf("EXAMPLE: ./sentinel eth0\n");
        #endif

        return -1;

    }else if(argc >= 2){

        initscr();
        printw("Hllo World");
        refresh();
        getch();
        endwin();

        receiver(argv[1]);
        return 0;

    }

}