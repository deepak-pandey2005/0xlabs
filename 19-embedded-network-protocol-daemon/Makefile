CC ?= gcc
CFLAGS ?= -Wall -Wextra -g -O0

all: mini_httpd

mini_httpd: mini_httpd.c
	$(CC) $(CFLAGS) mini_httpd.c -o mini_httpd

clean:
	rm -f mini_httpd
