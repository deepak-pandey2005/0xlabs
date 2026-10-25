/**
 * 0xLabs - Module 24: Uninitialized Memory, Struct Padding & Information Leaks
 *
 * Demonstrates:
 * 1. C structure natural alignment and implicit compiler padding holes.
 * 2. Uninitialized stack/kernel memory disclosure (the classic KASLR leak CVE pattern).
 * 3. Inspecting raw memory hex dumps to spot leaked stack pointers/canaries.
 * 4. Mitigations: memset(), value-initialization, and -ftrivial-auto-var-init.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

/*
 * Structure with deliberate compiler alignment padding:
 *
 * Member          Size   Offset
 * uint32_t id       4      0
 * [PADDING]         4      4   <-- 4 bytes padding to align uint64_t to 8-byte boundary
 * uint64_t value    8      8
 * uint16_t flag     2     16
 * [PADDING]         6     18   <-- 6 bytes trailing padding so sizeof(struct) is multiple of 8
 * Total Size: 24 bytes
 */
struct sensor_report {
    uint32_t id;
    uint64_t value;
    uint16_t flag;
};

#define SECRET_KERNEL_CANARY 0xDEADBEEFCAFEBA00ULL
#define SECRET_KASLR_POINTER 0xFFFFFFFF81001234ULL

/*
 * Simulated Kernel Stack Frame:
 * Leaves sensitive residual pointers on the stack before the report handler is called.
 */
__attribute__((noinline))
void populate_dirty_stack(void) {
    volatile uint64_t dirty_stack[8];
    dirty_stack[0] = SECRET_KERNEL_CANARY;
    dirty_stack[1] = SECRET_KASLR_POINTER;
    dirty_stack[2] = 0xAAAAAAAAAAAAAAAAULL;
    dirty_stack[3] = 0xBBBBBBBBBBBBBBBBULL;
    dirty_stack[4] = SECRET_KERNEL_CANARY;
    dirty_stack[5] = SECRET_KASLR_POINTER;
    dirty_stack[6] = 0xCCCCCCCCCCCCCCCCULL;
    dirty_stack[7] = 0xDDDDDDDDDDDDDDDDULL;
    (void)dirty_stack[0];
}

/* =========================================================================
 * VULNERABLE HANDLER: Partial field initialization
 *
 * Flaw: Only assigns 'id', 'value', and 'flag'.
 * The 10 bytes of padding (offsets 4..7 and 18..23) are never initialized!
 * When copied to user space, stack remnants are leaked.
 * ========================================================================= */
__attribute__((noinline))
void vulnerable_generate_report(struct sensor_report *out_report) {
    struct sensor_report r; /* Allocated on uninitialized stack */

    // Explicit field assignments:
    r.id = 0x1337;
    r.value = 42000;
    r.flag = 1;

    // FLAW: Copying sizeof(struct sensor_report) copies uninitialized padding bytes!
    memcpy(out_report, &r, sizeof(struct sensor_report));
}

/* =========================================================================
 * SECURE HANDLER: Full structure zeroing
 *
 * Defense: Clear entire struct memory including all compiler padding bytes
 * before populating fields.
 * ========================================================================= */
__attribute__((noinline))
void secure_generate_report(struct sensor_report *out_report) {
    struct sensor_report r;

    // Method 1: Explicit memset of entire structure
    memset(&r, 0, sizeof(struct sensor_report));

    r.id = 0x1337;
    r.value = 42000;
    r.flag = 1;

    memcpy(out_report, &r, sizeof(struct sensor_report));
}

static void print_hex_dump(const char *label, const uint8_t *buf, size_t len) {
    printf("%s (Size: %zu bytes):\n", label, len);
    for (size_t i = 0; i < len; i += 8) {
        printf("  Offset +%02zu: ", i);
        for (size_t j = i; j < i + 8 && j < len; j++) {
            printf("%02X ", buf[j]);
        }
        printf("\n");
    }
}

int main(int argc, char **argv) {
    printf("=== 0xLabs - Module 24: Struct Padding & Uninitialized Memory Leaks ===\n\n");
    printf("[*] Structural Analysis:\n");
    printf("    sizeof(uint32_t) id    = 4 bytes\n");
    printf("    sizeof(uint64_t) value = 8 bytes\n");
    printf("    sizeof(uint16_t) flag  = 2 bytes\n");
    printf("    Sum of members         = 14 bytes\n");
    printf("    sizeof(struct report)  = %zu bytes (10 bytes padding!)\n\n",
           sizeof(struct sensor_report));

    bool test_vuln = true;
    bool test_secure = true;

    if (argc > 1) {
        if (strcmp(argv[1], "--vuln") == 0) test_secure = false;
        else if (strcmp(argv[1], "--secure") == 0) test_vuln = false;
    }

    if (test_vuln) {
        printf("[TEST 1] Executing Vulnerable Handler (Partial Initialization)...\n");
        populate_dirty_stack();

        struct sensor_report report;
        memset(&report, 0, sizeof(report)); // Clean user receiver buffer
        vulnerable_generate_report(&report);

        uint8_t *raw = (uint8_t *)&report;
        print_hex_dump("[!] Raw bytes received from vulnerable handler", raw, sizeof(report));

        // Check for leaked canary or pointer in padding
        uint32_t padding1;
        memcpy(&padding1, &raw[4], sizeof(padding1));
        printf("\n    [ANALYSIS] Inspecting 4-byte padding hole at offset +04:\n");
        printf("    Padding bytes value: 0x%08X\n", padding1);

        if (padding1 != 0) {
            printf("    [CRITICAL LEAK] Uninitialized kernel/stack data detected in padding!\n");
            printf("    Residual stack canary / KASLR pointer leaked to user space!\n");
        } else {
            printf("    [INFO] Padding happened to be 0 on this run.\n");
        }
        printf("\n------------------------------------------------------------\n");
    }

    if (test_secure) {
        printf("[TEST 2] Executing Secure Handler (Full memset Zeroing)...\n");
        populate_dirty_stack();

        struct sensor_report report;
        memset(&report, 0xFF, sizeof(report)); // Fill user receiver buffer with 0xFF
        secure_generate_report(&report);

        uint8_t *raw = (uint8_t *)&report;
        print_hex_dump("[+] Raw bytes received from secure handler", raw, sizeof(report));

        uint32_t padding1;
        memcpy(&padding1, &raw[4], sizeof(padding1));
        printf("\n    [ANALYSIS] Inspecting 4-byte padding hole at offset +04:\n");
        printf("    Padding bytes value: 0x%08X\n", padding1);

        if (padding1 == 0) {
            printf("    [SECURE] Padding is cleanly zeroed! No memory disclosure possible.\n");
        } else {
            printf("    [FAILED] Padding contains non-zero data: 0x%08X\n", padding1);
        }
    }

    printf("\n============================================================\n");
    return 0;
}
