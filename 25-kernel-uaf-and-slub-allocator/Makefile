CC = cc
CFLAGS = -Wall -Wextra -g -O0

all: kernel_slub_mock

kernel_slub_mock: kernel_slub_mock.c
	$(CC) $(CFLAGS) kernel_slub_mock.c -o kernel_slub_mock

clean:
	rm -f kernel_slub_mock

.PHONY: all clean
