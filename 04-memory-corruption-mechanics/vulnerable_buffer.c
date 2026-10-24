#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
 * ============================================================================
 * MODULE 04: Memory Corruption Mechanics (Buffer Overwrite Theory)
 * ============================================================================
 * In C, arrays are contiguous blocks of memory without built-in runtime bounds checks.
 * When unbounded input functions (e.g., gets, strcpy, read without length checks)
 * are used, data overflows the buffer and overwrites adjacent memory on the stack:
 *
 * Stack Layout (High -> Low Address):
 *   [ Saved Return Address (8 bytes) ]  <-- Overwritten if buffer overflow >= offset
 *   [ Saved Base Pointer RBP (8 bytes) ]
 *   [ is_admin flag (4 bytes) ]         <-- Overwritten first by adjacent overflow
 *   [ buffer[16] (16 bytes) ]           <-- User input written here
 */

void check_access(void) {
    // Note: Variable ordering on stack depends on compiler, but variables
    // declared in a frame reside adjacent to each other.
    volatile int is_admin = 0;
    char buffer[16];

    printf("[*] Stack Memory Addresses:\n");
    printf("    buffer address   : %p\n", (void *)buffer);
    printf("    is_admin address : %p (distance from buffer: %ld bytes)\n", 
           (void *)&is_admin, (long)((char *)&is_admin - buffer));
    printf("    Initial is_admin value: 0x%08x (%d)\n\n", is_admin, is_admin);

    printf("Enter input string: ");
    fflush(stdout);

    // Unsafe unbounded read using fgets with oversized limit for demonstration
    // Allows observing byte corruption in GDB safely
    if (fgets(buffer, 64, stdin) == NULL) {
        return;
    }

    printf("\n[*] Post-Input State:\n");
    printf("    Buffer contents: \"%s\"\n", buffer);
    printf("    Current is_admin value: 0x%08x (%d)\n", is_admin, is_admin);

    if (is_admin != 0) {
        printf("[!] STATE CHANGE: is_admin was modified by adjacent stack overflow!\n");
    } else {
        printf("[+] Normal execution: is_admin remains unchanged.\n");
    }
}

int main(void) {
    printf("=== 0xLabs: Memory Corruption & Stack Mechanics ===\n\n");
    check_access();
    printf("[*] check_access() returned cleanly to main().\n");
    return 0;
}
