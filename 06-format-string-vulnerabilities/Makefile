CC ?= gcc
CFLAGS ?= -Wall -Wextra -g -O0 -no-pie -fno-stack-protector

all: format_vuln

format_vuln: format_vuln.c
	$(CC) $(CFLAGS) format_vuln.c -o format_vuln

clean:
	rm -f format_vuln
