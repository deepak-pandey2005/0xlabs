CC ?= gcc
CFLAGS ?= -Wall -Wextra -g -O0

all: integer_mechanics integer_mechanics_ubsan

integer_mechanics: integer_mechanics.c
	$(CC) $(CFLAGS) integer_mechanics.c -o integer_mechanics

integer_mechanics_ubsan: integer_mechanics.c
	$(CC) $(CFLAGS) -fsanitize=undefined integer_mechanics.c -o integer_mechanics_ubsan

clean:
	rm -f integer_mechanics integer_mechanics_ubsan
