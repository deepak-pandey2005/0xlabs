CC ?= gcc
CFLAGS ?= -Wall -Wextra -g -O0 -no-pie

all: target_v1 target_v2

target_v1: target_v1.c
	$(CC) $(CFLAGS) target_v1.c -o target_v1

target_v2: target_v2.c
	$(CC) $(CFLAGS) target_v2.c -o target_v2

clean:
	rm -f target_v1 target_v2 v1_disasm.txt v2_disasm.txt patch_diff.diff
