CC ?= gcc
CFLAGS ?= -Wall -Wextra -g -O0 -no-pie

all: flattened_target

flattened_target: flattened_target.c
	$(CC) $(CFLAGS) flattened_target.c -o flattened_target

clean:
	rm -f flattened_target
