#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
 * ============================================================================
 * MODULE 10: Dynamic Linking, PLT/GOT & API Hooking
 * ============================================================================
 * This application uses standard C library functions (strcmp, puts) which
 * are dynamically resolved via the Procedure Linkage Table (PLT) and
 * Global Offset Table (GOT).
 */

int authenticate(const char *user_key) {
    // Secret stored in binary; compared with strcmp
    const char *REAL_KEY = "SUPER_SECRET_0xKEY_2026";
    return strcmp(user_key, REAL_KEY) == 0;
}

int main(int argc, char *argv[]) {
    printf("=== 0xLabs: Dynamic Linking & Hooking Demo ===\n");

    if (argc < 2) {
        printf("Usage: %s <key>\n", argv[0]);
        return 1;
    }

    if (authenticate(argv[1])) {
        printf("[SUCCESS] Access Granted! Key accepted.\n");
        return 0;
    } else {
        printf("[FAILED] Access Denied. Key incorrect.\n");
        return 1;
    }
}
