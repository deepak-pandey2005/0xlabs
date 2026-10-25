/**
 * 0xLabs - Module 27: Symbolic Execution & Constraint Solving
 *
 * A binary authentication challenge with non-linear multi-variable constraints.
 * Demonstrates:
 * 1. How compilers translate algebraic conditions into machine code.
 * 2. Why manual brute-forcing is computationally impractical ($2^{32}$ states).
 * 3. How symbolic execution extracts path constraints and solves for the key.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

bool verify_key(const char *key) {
    if (strlen(key) < 4) {
        return false;
    }

    uint8_t k0 = (uint8_t)key[0];
    uint8_t k1 = (uint8_t)key[1];
    uint8_t k2 = (uint8_t)key[2];
    uint8_t k3 = (uint8_t)key[3];

    // Constraint 1: Bitwise XOR relation
    if ((k0 ^ 0x1E) != 0x55) {
        return false;
    }

    // Constraint 2: Affine transformation
    if ((k1 * 5 + 17) != 362) {
        return false;
    }

    // Constraint 3: Cross-variable interdependency
    if ((k2 ^ k0) != 0x12) {
        return false;
    }

    // Constraint 4: Multi-variable linear relation
    if ((k3 * 4 + k1) != 201) {
        return false;
    }

    return true;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s <4-character-key>\n", argv[0]);
        return 1;
    }

    const char *user_key = argv[1];
    printf("[*] Testing key: '%s'...\n", user_key);

    if (verify_key(user_key)) {
        printf("[+] SUCCESS: Access Granted! Token: 0xLABS-SYMBOLIC-SOLVED\n");
        return 0;
    } else {
        printf("[-] FAILURE: Access Denied! Invalid key.\n");
        return 1;
    }
}
