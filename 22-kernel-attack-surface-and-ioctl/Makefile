CC = cc
CFLAGS = -Wall -Wextra -g -O0

all: mock_kernel_ioctl

mock_kernel_ioctl: mock_kernel_ioctl.c
	$(CC) $(CFLAGS) mock_kernel_ioctl.c -o mock_kernel_ioctl

clean:
	rm -f mock_kernel_ioctl

.PHONY: all clean
