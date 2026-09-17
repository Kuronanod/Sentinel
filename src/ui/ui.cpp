#ifdef _WIN32
    #include "curses.h"
#elif defined(__linux__)
    #include <ncurses.h>
#else
    #error "OS Not Supported"

#endif

int main(){

    initscr();
    printw("HELLO");
    refresh();
    getch();
    endwin();

    return 0;
}