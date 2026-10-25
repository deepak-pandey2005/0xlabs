CC = cc
CFLAGS = -Wall -Wextra -g -O0 -fno-stack-protector

all: infoleak_lab infoleak_auto_zero

infoleak_lab: infoleak_lab.c
	$(CC) $(CFLAGS) infoleak_lab.c -o infoleak_lab

infoleak_auto_zero: infoleak_lab.c
	$(CC) $(CFLAGS) -ftrivial-auto-var-init=zero infoleak_lab.c -o infoleak_auto_zero

clean:
	rm -f infoleak_lab infoleak_auto_zero

.PHONY: all clean
