CC ?= gcc
CFLAGS ?= -Wall -Wextra

MODULES := 01-binary-anatomy 02-x86_64-and-stack-frames 03-reverse-engineering-basics 04-memory-corruption-mechanics 05-binary-mitigations-and-defense 06-format-string-vulnerabilities 07-heap-internals-and-lifecycle 08-rop-theory-and-cfi 09-integer-overflow-and-signedness 10-dynamic-linking-and-hooking 11-syscalls-and-process-isolation 12-coverage-guided-fuzzing

.PHONY: all clean $(MODULES)

all: $(MODULES)

$(MODULES):
	$(MAKE) -C $@

clean:
	@for dir in $(MODULES); do \
		$(MAKE) -C $$dir clean; \
	done
	@echo "[+] Cleaned all lab modules."
