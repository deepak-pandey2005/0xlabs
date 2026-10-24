#include <stdio.h>
#include <stdlib.h>

/*
 * ============================================================================
 * MODULE 08: Return-Oriented Programming (ROP) Theory & Control Flow Integrity
 * ============================================================================
 * Concepts:
 *   1. Why NX/DEP led to Code-Reuse attacks (Ret2libc & ROP).
 *   2. What is a "Gadget"? An instruction sequence ending in 'ret' (opcode 0xc3).
 *   3. How 'pop rdi; ret' prepares the 1st argument for x86_64 functions.
 *   4. Modern mitigations: CET Shadow Stack, IBT (endbr64), Clang CFI, ARM PAC.
 */

// Target functions representing distinct operations
void print_banner(void) {
    printf("[+] Step 1: Banner displayed successfully.\n");
}

void process_argument(unsigned long arg) {
    printf("[+] Step 2: process_argument called with argument = 0x%lx\n", arg);
}

void finalize_workflow(void) {
    printf("[+] Step 3: Workflow finalized securely.\n");
}

// Deliberately included gadget-like helper functions to illustrate how compilers
// naturally emit register-popping sequences before returns
__attribute__((noinline))
void helper_gadget_rdi(unsigned long val) {
    // When disassembled, notice how the compiler loads/stores registers
    // followed by 'ret'
    (void)val;
}

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    printf("=== 0xLabs: ROP Gadget Mechanics & Control Flow Theory ===\n");
    print_banner();
    process_argument(0x1337);
    finalize_workflow();

    return 0;
}
