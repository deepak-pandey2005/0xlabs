CC ?= gcc
CFLAGS ?= -Wall -Wextra -g -O1 -fsanitize=address

all: bmp_decoder

bmp_decoder: bmp_decoder.c
	$(CC) $(CFLAGS) bmp_decoder.c -o bmp_decoder

clean:
	rm -f bmp_decoder sample.bmp crafted_overflow.bmp
