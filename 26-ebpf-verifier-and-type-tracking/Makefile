CC = cc
CFLAGS = -Wall -Wextra -g -O0

all: ebpf_verifier_mock

ebpf_verifier_mock: ebpf_verifier_mock.c
	$(CC) $(CFLAGS) ebpf_verifier_mock.c -o ebpf_verifier_mock

clean:
	rm -f ebpf_verifier_mock

.PHONY: all clean
