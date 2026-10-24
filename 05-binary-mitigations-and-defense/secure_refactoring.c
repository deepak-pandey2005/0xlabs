#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

/*
 * ============================================================================
 * MODULE 05: Secure Systems Programming & Defensive Refactoring
 * ============================================================================
 * Principles Demonstrated:
 *   1. Strict Bounds Enforcement: Never read more than sizeof(destination).
 *   2. Use sizeof() consistently rather than hardcoded magic numbers.
 *   3. Always validate return values of I/O functions.
 *   4. Strip newline / sanitize untrusted inputs.
 *   5. Compiler Hardening: Compiled with -fstack-protector-strong, -fPIE,
 *      -Wl,-z,relro,-z,now, -D_FORTIFY_SOURCE=2.
 */

#define MAX_BUFFER_SIZE 16

bool read_safe_string(char *dest, size_t dest_size) {
    if (dest == NULL || dest_size == 0) {
        return false;
    }

    // Bounds-safe input using fgets with exact destination capacity
    if (fgets(dest, (int)dest_size, stdin) == NULL) {
        dest[0] = '\0';
        return false;
    }

    // Strip trailing newline safely
    size_t len = strlen(dest);
    if (len > 0 && dest[len - 1] == '\n') {
        dest[len - 1] = '\0';
    } else {
        // If no newline was captured, excess input remains in the stream;
        // flush the input buffer to prevent stream pollution.
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF);
    }

    return true;
}

void safe_operation(void) {
    int safe_counter = 1337;
    char safe_buffer[MAX_BUFFER_SIZE];

    printf("[*] Enter bounded input (max %zu characters): ", sizeof(safe_buffer) - 1);
    fflush(stdout);

    if (read_safe_string(safe_buffer, sizeof(safe_buffer))) {
        printf("[+] Safely read input: \"%s\"\n", safe_buffer);
        printf("[+] Integrity preserved! safe_counter = %d (address: %p)\n", 
               safe_counter, (void *)&safe_counter);
    } else {
        printf("[-] Failed to read input safely.\n");
    }
}

int main(void) {
    printf("=== 0xLabs: Secure Refactoring Demonstration ===\n");
    safe_operation();
    printf("[+] Operation completed securely.\n");
    return 0;
}
