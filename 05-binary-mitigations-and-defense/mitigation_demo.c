#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
 * ============================================================================
 * MODULE 05: Modern Binary Mitigations & Defense
 * ============================================================================
 * Demonstrating how Stack Canaries detect memory corruption at runtime
 * and abort before the return address can be hijacked.
 */

void test_canary(void) {
    char buffer[16];

    printf("[*] Enter input to test canary detection: ");
    fflush(stdout);

    // Read up to 64 bytes into a 16-byte buffer
    if (fgets(buffer, 64, stdin) == NULL) {
        return;
    }

    printf("[*] Buffer received: %s\n", buffer);
    printf("[*] Reached end of function. About to execute epilogue & canary check...\n");
}

int main(void) {
    printf("=== 0xLabs: Stack Canary & Mitigation Demonstration ===\n");
    test_canary();
    printf("[+] Returned safely to main().\n");
    return 0;
}
