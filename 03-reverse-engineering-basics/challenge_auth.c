#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/*
 * ============================================================================
 * MODULE 03: Reverse Engineering Basics - Authentication Logic
 * ============================================================================
 * Objective: Determine the valid license key without reading this source code,
 * using reverse engineering tools: strings, objdump, and gdb.
 */

// Target hash / transformed values:
// Plaintext: "0xLABS-2026"
// Obfuscation: char[i] ^ (0x5A + i)
static const unsigned char EXPECTED_TRANSFORM[] = {
    0x6a, 0x23, 0x10, 0x1c, 0x1c, 0x0c, 0x4d, 0x53, 0x52, 0x51, 0x52
};
#define EXPECTED_LEN 11

static bool verify_key(const char *input) {
    if (strlen(input) != EXPECTED_LEN) {
        return false;
    }

    for (size_t i = 0; i < EXPECTED_LEN; i++) {
        unsigned char transformed = (unsigned char)input[i] ^ (unsigned char)(0x5A + i);
        if (transformed != EXPECTED_TRANSFORM[i]) {
            return false;
        }
    }

    return true;
}

int main(int argc, char *argv[]) {
    printf("=== 0xLabs: Reverse Engineering Challenge ===\n");

    if (argc < 2) {
        printf("Usage: %s <LICENSE_KEY>\n", argv[0]);
        return 1;
    }

    if (verify_key(argv[1])) {
        printf("[SUCCESS] Access Granted! Authentication Key Validated.\n");
        return 0;
    } else {
        printf("[FAILED] Invalid Key. Access Denied.\n");
        return 1;
    }
}
