CC ?= gcc
CFLAGS ?= -Wall -Wextra -g -O0

all: toctou_demo

toctou_demo: toctou_demo.c
	$(CC) $(CFLAGS) toctou_demo.c -o toctou_demo

clean:
	rm -f toctou_demo target.txt protected_file.txt test_symlink
