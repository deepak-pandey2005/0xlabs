CC ?= gcc
CFLAGS ?= -Wall -Wextra -g -O0

all: anatomy anatomy_stripped

anatomy: anatomy.c
	$(CC) $(CFLAGS) -no-pie anatomy.c -o anatomy

anatomy_stripped: anatomy
	cp anatomy anatomy_stripped
	strip --strip-all anatomy_stripped

clean:
	rm -f anatomy anatomy_stripped
