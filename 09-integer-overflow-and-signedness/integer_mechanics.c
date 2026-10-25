#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include <stdbool.h>

/*
 * ============================================================================
 * MODULE 09: Integer Overflow, Truncation & Signedness Mechanics
 * ============================================================================
 * In x86_64 and C, integer vulnerabilities arise from:
 *   1. Arithmetic Wrap-around: Exceeding maximum representable value.
 *   2. Integer Truncation: Downcasting wider types (uint64_t -> uint16_t).
 *   3. Sign Extension / Signedness Confusion: Treating signed values as unsigned.
 *   4. Undersized Allocation: (count * size) wraps, leading to heap out-of-bounds.
 */

// 1. Signed vs Unsigned Representation & CPU Flags
void demo_cpu_flags(void) {
    printf("=== Demonstration 1: Hardware Arithmetic Wrap-around ===\n");

    // Unsigned 32-bit wrap
    uint32_t max_u32 = UINT32_MAX; // 0xFFFFFFFF (4,294,967,295)
    uint32_t wrapped_u32 = max_u32 + 1;
    printf("[*] Unsigned 32-bit:\n");
    printf("    UINT32_MAX     : %u (0x%08x)\n", max_u32, max_u32);
    printf("    UINT32_MAX + 1 : %u (0x%08x) -> CPU Carry Flag (CF) set\n\n", wrapped_u32, wrapped_u32);

    // Signed 32-bit overflow
    int32_t max_s32 = INT32_MAX; // 0x7FFFFFFF (+2,147,483,647)
    // In C standard, signed integer overflow is Undefined Behavior (UB),
    // but in x86_64 two's complement hardware it wraps to negative.
    int32_t wrapped_s32 = (int32_t)((uint32_t)max_s32 + 1);
    printf("[*] Signed 32-bit (Two's Complement):\n");
    printf("    INT32_MAX      : %d (0x%08x)\n", max_s32, max_s32);
    printf("    INT32_MAX + 1  : %d (0x%08x) -> CPU Overflow Flag (OF) set\n\n", wrapped_s32, wrapped_s32);
}

// 2. The Classic Undersized Allocation Bug
void demo_undersized_allocation(uint32_t num_items) {
    printf("=== Demonstration 2: Undersized Allocation (Multiplication Overflow) ===\n");
    printf("[*] Requested items count: %u\n", num_items);
    printf("[*] Size of each item     : %zu bytes\n", sizeof(uint32_t));

    // VULNERABLE: 32-bit arithmetic wraps around before malloc
    uint32_t total_bytes = num_items * (uint32_t)sizeof(uint32_t);
    printf("[!] Calculated allocation size: %u * %zu = %u bytes (0x%08x)\n",
           num_items, sizeof(uint32_t), total_bytes, total_bytes);

    if (total_bytes < num_items) {
        printf("[!] ARITHMETIC OVERFLOW DETECTED: total_bytes wrapped around!\n");
        printf("    Buffer requested: %u bytes, but caller expects storage for %u items (%lu bytes)!\n\n",
               total_bytes, num_items, (unsigned long)num_items * sizeof(uint32_t));
    } else {
        printf("[+] Arithmetic did not wrap.\n\n");
    }
}

// 3. Signedness Confusion in Bounds Checking
void demo_signedness_confusion(int length_request) {
    printf("=== Demonstration 3: Signed Comparison vs Unsigned Size ===\n");
    printf("[*] Input length request: %d\n", length_request);

    #define MAX_ALLOWED 1024
    char destination_buffer[MAX_ALLOWED];
    (void)destination_buffer;

    // VULNERABLE CHECK: Signed comparison
    // If length_request is negative (e.g. -1), length_request < MAX_ALLOWED is TRUE!
    if (length_request < MAX_ALLOWED) {
        printf("[+] Check passed: length_request (%d) < MAX_ALLOWED (%d)\n", length_request, MAX_ALLOWED);
        
        // However, standard memory functions (memcpy, fread, read) take size_t (unsigned 64-bit)!
        size_t casted_size = (size_t)length_request;
        printf("[!] WARNING: When passed to memcpy(..., size_t count):\n");
        printf("    (size_t)(%d) becomes: %zu (0x%016zx) bytes!\n", length_request, casted_size, casted_size);
        printf("    This causes a massive multi-gigabyte copy attempt leading to a SIGSEGV!\n\n");
    } else {
        printf("[-] Rejected: length exceeds MAX_ALLOWED\n\n");
    }
}

// 4. Secure Arithmetic Pattern using Compiler Builtins
void demo_safe_multiplication(uint32_t num_items) {
    printf("=== Demonstration 4: Safe Arithmetic with Compiler Built-ins ===\n");
    uint32_t element_size = sizeof(uint32_t);
    uint32_t total_bytes = 0;

    // Use GCC/Clang hardware-checked overflow builtin:
    // Returns true if overflow occurred; otherwise writes product to total_bytes
    if (__builtin_mul_overflow(num_items, element_size, &total_bytes)) {
        printf("[+] DEFENSIVE SUCCESS: __builtin_mul_overflow detected wrap-around!\n");
        printf("    Safely rejecting allocation for %u elements to prevent heap corruption.\n\n", num_items);
    } else {
        printf("[+] Safe allocation: %u bytes successfully verified.\n\n", total_bytes);
    }
}

int main(int argc, char *argv[]) {
    printf("=== 0xLabs: Integer Overflow & Signedness Mechanics ===\n\n");

    demo_cpu_flags();

    // Trigger multiplication overflow: 0x40000001 * 4 = 0x100000004 -> wraps to 4 bytes in 32-bit!
    uint32_t overflow_count = 0x40000001;
    demo_undersized_allocation(overflow_count);

    // Trigger signedness confusion with negative index
    demo_signedness_confusion(-1);

    // Demonstrate defensive overflow prevention
    demo_safe_multiplication(overflow_count);

    (void)argc;
    (void)argv;
    return 0;
}
