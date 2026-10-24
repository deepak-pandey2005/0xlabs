CC ?= gcc
# Disable omit-frame-pointer to ensure standard rbp base pointer frames for clarity
# Disable stack-protector here specifically so students inspect pure stack frames before Module 05
CFLAGS ?= -Wall -Wextra -g -O0 -fno-omit-frame-pointer -fno-stack-protector -no-pie

all: stack_frame stack_frame.s

stack_frame: stack_frame.c
	$(CC) $(CFLAGS) stack_frame.c -o stack_frame

stack_frame.s: stack_frame.c
	$(CC) $(CFLAGS) -S -masm=intel stack_frame.c -o stack_frame.s

clean:
	rm -f stack_frame stack_frame.s
