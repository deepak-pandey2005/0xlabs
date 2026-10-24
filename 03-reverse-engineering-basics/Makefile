CC ?= gcc
CFLAGS ?= -Wall -Wextra -O0 -no-pie

all: challenge_auth challenge_auth_stripped

challenge_auth: challenge_auth.c
	$(CC) $(CFLAGS) -g challenge_auth.c -o challenge_auth

challenge_auth_stripped: challenge_auth.c
	$(CC) $(CFLAGS) -s challenge_auth.c -o challenge_auth_stripped

clean:
	rm -f challenge_auth challenge_auth_stripped
