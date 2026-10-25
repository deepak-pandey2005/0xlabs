CC = cc
CFLAGS = -Wall -Wextra -g -O0 -no-pie

all: auth_challenge

auth_challenge: auth_challenge.c
	$(CC) $(CFLAGS) auth_challenge.c -o auth_challenge

clean:
	rm -f auth_challenge

.PHONY: all clean
