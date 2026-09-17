CC = gcc
CFLAGS = -O2 -Wall
INCLUDES = -I./include -I./include/curses

WIN_LIBS = -L./lib -lpdcurses -lws2_32
WIN_OUT = sentinel.exe

LINUX_LIBS = -lncurses
LINUX_OUT = sentinel

SRCS = src/main.c \
		src/receiver/receiver.c \
		src/receiver/parser.c \
		src/ui/ui.c

ifeq ($(OS),Windows_NT)
	LIBS = $(WIN_LIBS)
	OUT = $(WIN_OUT)
	SRCS += src/receiver/windows/socket.c \
			src/receiver/windows/get_packet.c \
			src/receiver/windows/read.c

else ifeq ($(shell uname),Linux)
	LIBS = $(LINUX_LIBS)
	OUT = $(LINUX_OUT)
	SRCS += src/receiver/linux/socket.c \
			src/receiver/linux/ring_buffer.c \
			src/receiver/linux/read.c

else
	$(error OS Not Supported)
endif

all:
	$(CC) $(CFLAGS) $(INCLUDES) $(SRCS) $(LIBS) -o $(OUT)

clean:
	rm -f $(OUT)