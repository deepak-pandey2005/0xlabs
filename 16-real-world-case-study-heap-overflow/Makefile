CC ?= gcc
CFLAGS ?= -Wall -Wextra -g -O1 -fsanitize=address

all: cli_unescape

cli_unescape: cli_unescape.c
	$(CC) $(CFLAGS) cli_unescape.c -o cli_unescape

clean:
	rm -f cli_unescape
