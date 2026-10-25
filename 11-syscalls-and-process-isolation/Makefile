CC ?= gcc
CFLAGS ?= -Wall -Wextra -g -O0

all: syscall_demo seccomp_sandbox

syscall_demo: syscall_demo.c
	$(CC) $(CFLAGS) syscall_demo.c -o syscall_demo

seccomp_sandbox: seccomp_sandbox.c
	$(CC) $(CFLAGS) seccomp_sandbox.c -o seccomp_sandbox

clean:
	rm -f syscall_demo seccomp_sandbox
