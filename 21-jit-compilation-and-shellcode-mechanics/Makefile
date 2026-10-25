CC ?= gcc
CFLAGS ?= -Wall -Wextra -g -O0

all: jit_engine

jit_engine: jit_engine.c
	$(CC) $(CFLAGS) jit_engine.c -o jit_engine

clean:
	rm -f jit_engine
