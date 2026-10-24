CC ?= gcc
CFLAGS ?= -Wall -Wextra -g -O0 -no-pie

all: heap_demo heap_demo_asan

heap_demo: heap_demo.c
	$(CC) $(CFLAGS) heap_demo.c -o heap_demo

heap_demo_asan: heap_demo.c
	$(CC) $(CFLAGS) -fsanitize=address heap_demo.c -o heap_demo_asan

clean:
	rm -f heap_demo heap_demo_asan
