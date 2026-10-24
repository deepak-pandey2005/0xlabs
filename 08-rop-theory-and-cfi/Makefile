CC ?= gcc
CFLAGS ?= -Wall -Wextra -g -O0 -no-pie -fno-stack-protector

all: rop_gadget_inspector

rop_gadget_inspector: rop_gadget_inspector.c
	$(CC) $(CFLAGS) rop_gadget_inspector.c -o rop_gadget_inspector

clean:
	rm -f rop_gadget_inspector
