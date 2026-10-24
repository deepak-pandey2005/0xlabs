CC ?= gcc
# Disable stack protector and optimizations to isolate pure memory layout
CFLAGS ?= -Wall -Wextra -g -O0 -fno-stack-protector -fno-omit-frame-pointer -no-pie

all: vulnerable_buffer

vulnerable_buffer: vulnerable_buffer.c
	$(CC) $(CFLAGS) vulnerable_buffer.c -o vulnerable_buffer

clean:
	rm -f vulnerable_buffer
