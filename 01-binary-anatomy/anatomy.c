#include <stdio.h>
#include <stdlib.h>

/*
 * ============================================================================
 * MODULE 01: ELF Binary Anatomy
 * ============================================================================
 * This program demonstrates how different variables and functions in C
 * are organized into specific sections of an ELF (Executable and Linkable Format)
 * binary file.
 */

// 1. .rodata Section (Read-Only Data)
// String literals and const-qualified global variables are stored here.
// Attempting to write to this memory segment triggers a SIGSEGV at runtime.
const char * const RO_GREETING = "=== 0xLabs: ELF Binary Anatomy ===";
const int RO_VERSION_ID = 0x1337;

// 2. .data Section (Initialized Global/Static Data)
// Variables that have explicit non-zero initialization values at compile time.
// These take up actual space on disk inside the ELF file.
int g_initialized_counter = 42;
char g_platform_name[] = "Linux x86_64";

// 3. .bss Section (Block Started by Symbol - Uninitialized Data)
// Global/static variables that are uninitialized or explicitly initialized to 0.
// They DO NOT take up space inside the binary on disk; the ELF header specifies
// how many bytes the kernel must allocate and zero out at load time.
int g_uninitialized_buffer[1024]; // 4096 bytes reserved in BSS
int g_zero_initialized = 0;

// 4. .text Section (Executable Machine Code)
// Compiled CPU instructions reside in .text, marked with Read & Execute (R-X) permissions.
void print_memory_layout(void) {
    // Local stack variables
    int stack_var = 100;
    
    // Heap allocation
    void *heap_var = malloc(64);

    printf("%s\n", RO_GREETING);
    printf("[*] Section Address Inspection (Virtual Memory Map):\n");
    printf("    .text   (Code / Function)      : %p\n", (void *)print_memory_layout);
    printf("    .rodata (Read-Only String)     : %p\n", (void *)RO_GREETING);
    printf("    .data   (Initialized Global)   : %p\n", (void *)&g_initialized_counter);
    printf("    .bss    (Uninitialized Global) : %p\n", (void *)g_uninitialized_buffer);
    printf("    Heap    (malloc allocated)     : %p\n", heap_var);
    printf("    Stack   (Local Variable)       : %p\n", (void *)&stack_var);

    free(heap_var);
}

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    print_memory_layout();
    return 0;
}
