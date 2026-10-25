#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

/*
 * ============================================================================
 * MODULE 15: Control Flow Flattening (CFF) & Reverse Engineering De-obfuscation
 * ============================================================================
 * Control Flow Flattening transforms linear logic into a state machine
 * inside a switch-dispatcher loop. This destroys the natural control flow
 * graph (CFG) in disassemblers (IDA/Ghidra).
 */

// 1. Original Unobfuscated Logic (Linear Flow)
int compute_hash_original(const char *input) {
    int hash = 0x1337;
    int len = strlen(input);

    for (int i = 0; i < len; i++) {
        hash = (hash ^ input[i]) + 0x5a;
    }

    if (hash % 2 == 0) {
        hash += 42;
    } else {
        hash -= 13;
    }

    return hash;
}

// 2. Control Flow Flattened Logic (State Machine Dispatcher)
// All basic blocks are flattened into cases inside a central loop.
int compute_hash_flattened(const char *input) {
    int hash;
    int len;
    int i;
    
    // State variable controlling the dispatcher
    uint32_t state = 0x10; 

    while (state != 0x99) { // 0x99 = TERMINATE state
        switch (state) {
            case 0x10: // Basic Block: Initialization
                hash = 0x1337;
                len = strlen(input);
                i = 0;
                state = 0x20; // Transition to loop condition
                break;

            case 0x20: // Basic Block: Loop Condition Check
                if (i < len) {
                    state = 0x30; // Body
                } else {
                    state = 0x40; // Loop exit / branch
                }
                break;

            case 0x30: // Basic Block: Loop Body
                hash = (hash ^ input[i]) + 0x5a;
                i++;
                state = 0x20; // Back to condition check
                break;

            case 0x40: // Basic Block: Conditional Branch
                if (hash % 2 == 0) {
                    state = 0x50;
                } else {
                    state = 0x60;
                }
                break;

            case 0x50: // Basic Block: Even branch
                hash += 42;
                state = 0x99; // Terminate
                break;

            case 0x60: // Basic Block: Odd branch
                hash -= 13;
                state = 0x99; // Terminate
                break;

            default:
                state = 0x99;
                break;
        }
    }

    return hash;
}

int main(int argc, char *argv[]) {
    const char *test_input = (argc > 1) ? argv[1] : "0xLabs";

    printf("=== 0xLabs: Control Flow Flattening De-obfuscation Target ===\n");
    printf("[*] Input string: \"%s\"\n\n", test_input);

    int res_orig = compute_hash_original(test_input);
    int res_flat = compute_hash_flattened(test_input);

    printf("[+] Original Linear Result   : 0x%08x (%d)\n", res_orig, res_orig);
    printf("[+] Flattened Dispatch Result: 0x%08x (%d)\n", res_flat, res_flat);

    if (res_orig == res_flat) {
        printf("[✓] Functional equivalence verified!\n");
    }

    return 0;
}
