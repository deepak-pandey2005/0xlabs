CC := clang
CFLAGS := -Wall -Wextra -g -O1

all: packet_fuzzer standalone_reproducer

packet_fuzzer: packet_parser.c fuzz_target.c packet_parser.h
	$(CC) $(CFLAGS) -fsanitize=fuzzer,address packet_parser.c fuzz_target.c -o packet_fuzzer

standalone_reproducer: packet_parser.c standalone_reproducer.c packet_parser.h
	$(CC) $(CFLAGS) -fsanitize=address packet_parser.c standalone_reproducer.c -o standalone_reproducer

clean:
	rm -f packet_fuzzer standalone_reproducer crash-* leak-* timeout-*
	rm -rf corpus/
