CC ?= gcc
CFLAGS_COMMON := -Wall -Wextra -g

all: demo_no_canary demo_with_canary demo_hardened secure_refactoring

demo_no_canary: mitigation_demo.c
	$(CC) $(CFLAGS_COMMON) -O0 -fno-stack-protector -no-pie -z execstack mitigation_demo.c -o demo_no_canary

demo_with_canary: mitigation_demo.c
	$(CC) $(CFLAGS_COMMON) -O0 -fstack-protector-all -no-pie mitigation_demo.c -o demo_with_canary

demo_hardened: mitigation_demo.c
	$(CC) $(CFLAGS_COMMON) -O2 -fstack-protector-strong -fPIE -pie -Wl,-z,relro,-z,now -D_FORTIFY_SOURCE=2 mitigation_demo.c -o demo_hardened

secure_refactoring: secure_refactoring.c
	$(CC) $(CFLAGS_COMMON) -O2 -fstack-protector-strong -fPIE -pie -Wl,-z,relro,-z,now -D_FORTIFY_SOURCE=2 secure_refactoring.c -o secure_refactoring

clean:
	rm -f demo_no_canary demo_with_canary demo_hardened secure_refactoring
