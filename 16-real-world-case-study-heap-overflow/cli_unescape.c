#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/*
 * ============================================================================
 * MODULE 16: Real-World Case Study - Argument Unescaping Heap Overflow
 * ============================================================================
 * Pattern inspired by notable vulnerabilities (e.g. CVE-2021-3156):
 * When parsing command-line flags or escaped parameters, developers often
 * write custom string-unescaping loops.
 *
 * The Flaw:
 *   If an argument ends with a trailing backslash ('\\'), incrementing the
 *   source pointer to skip the escape character leaps OVER the terminating
 *   null byte ('\0')!
 *   The loop continues reading past the source buffer and writing past the
 *   allocated destination buffer on the heap!
 */

// 1. VULNERABLE UNESCAPING LOGIC
char *unescape_argument_vulnerable(const char *arg) {
    size_t len = strlen(arg);
    // Allocate buffer just large enough for the unescaped string
    char *dest = (char *)malloc(len + 1);
    if (!dest) return NULL;

    const char *src = arg;
    char *d = dest;

    while (*src != '\0') {
        if (*src == '\\') {
            // BUG: If *src is '\\' and the very next character is '\0' (trailing slash),
            // src++ advances past '\\', pointing to '\0'.
            // Then in the assignment below, *src++ copies '\0' and advances PAST THE END!
            src++;
        }
        *d++ = *src++;
    }
    *d = '\0';

    return dest;
}

// 2. PATCHED SECURE LOGIC
char *unescape_argument_patched(const char *arg) {
    size_t len = strlen(arg);
    char *dest = (char *)malloc(len + 1);
    if (!dest) return NULL;

    const char *src = arg;
    char *d = dest;

    while (*src != '\0') {
        // DEFENSIVE FIX: Check that the character following '\\' is NOT the null terminator!
        if (*src == '\\' && *(src + 1) != '\0') {
            src++;
        }
        *d++ = *src++;
    }
    *d = '\0';

    return dest;
}

int main(int argc, char *argv[]) {
    printf("=== 0xLabs: Real-World Case Study (String Unescaping Heap Overflow) ===\n");

    bool use_patch = false;
    const char *input = "normal\\ arg";

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--patch") == 0) {
            use_patch = true;
        } else {
            input = argv[i];
        }
    }

    printf("[*] Mode  : %s\n", use_patch ? "SECURE (Patched)" : "VULNERABLE");
    printf("[*] Input : \"%s\" (Length: %zu)\n\n", input, strlen(input));

    char *result = NULL;
    if (use_patch) {
        result = unescape_argument_patched(input);
    } else {
        result = unescape_argument_vulnerable(input);
    }

    if (result) {
        printf("[+] Output: \"%s\"\n", result);
        free(result);
    }

    return 0;
}
