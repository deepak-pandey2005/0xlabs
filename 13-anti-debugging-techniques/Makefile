CC ?= gcc
CFLAGS ?= -Wall -Wextra -g -O0 -no-pie

all: anti_debug

anti_debug: anti_debug.c
	$(CC) $(CFLAGS) anti_debug.c -o anti_debug

clean:
	rm -f anti_debug
