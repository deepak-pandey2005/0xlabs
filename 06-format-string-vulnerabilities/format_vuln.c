#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
 * ============================================================================
 * MODULE 06: Format String Vulnerabilities & Memory Disclosure
 * ============================================================================
 * In C, printf-family functions are variadic (take a variable number of args).
 * The caller places arguments in registers (%rsi, %rdx, ...) and the stack.
 *
 * If an application calls printf(user_input) directly instead of
 * printf("%s", user_input), the printf parser interprets any '%' characters
 * as conversion specifiers, reading values directly from caller registers
 * and stack memory slots!
 */

void vulnerable_logger(void) {
    // Secret / sensitive data residing on the local stack frame
    volatile unsigned long secret_token = 0xDEADBEEFCAFE1337ULL;
    char user_input[128];

    printf("[*] Sensitive token resides on stack at: %p\n", (void *)&secret_token);
    printf("[*] Enter message to log: ");
    fflush(stdout);

    if (fgets(user_input, sizeof(user_input), stdin) == NULL) {
        return;
    }

    // Strip newline
    user_input[strcspn(user_input, "\n")] = '\0';

    printf("\n--- Unsafe Logging Output ---\n");
    // VULNERABLE: Direct user-controlled format string
    // Suppress compiler warning with pragma specifically for this demonstration
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-security"
    printf(user_input);
#pragma GCC diagnostic pop
    printf("\n-----------------------------\n");
}

void safe_logger(void) {
    volatile unsigned long secret_token = 0xDEADBEEFCAFE1337ULL;
    char user_input[128];

    printf("[*] Enter message for safe logging: ");
    fflush(stdout);

    if (fgets(user_input, sizeof(user_input), stdin) == NULL) {
        return;
    }

    user_input[strcspn(user_input, "\n")] = '\0';

    printf("\n--- Safe Logging Output ---\n");
    // SECURE: Format string is a static literal; user input is passed as data
    printf("%s", user_input);
    printf("\n---------------------------\n");
    (void)secret_token;
}

int main(int argc, char *argv[]) {
    printf("=== 0xLabs: Format String Vulnerability & Memory Disclosure ===\n\n");

    if (argc > 1 && strcmp(argv[1], "--safe") == 0) {
        safe_logger();
    } else {
        vulnerable_logger();
    }

    return 0;
}
