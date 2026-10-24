#include <stdio.h>

/*
 * ============================================================================
 * MODULE 02: x86_64 Calling Conventions and Stack Frames
 * ============================================================================
 * System V AMD64 ABI Calling Convention:
 *   - Arguments 1-6 are passed in registers:
 *       1: %rdi, 2: %rsi, 3: %rdx, 4: %rcx, 5: %r8, 6: %r9
 *   - Arguments 7+ are passed on the stack.
 *   - Return value is placed in %rax.
 *   - Stack must be 16-byte aligned before any 'call' instruction.
 */

__attribute__((noinline))
int compute_sum(int a, int b, int c, int d, int e, int f, int g) {
    // Local stack variables
    int local_temp = 0x1111;
    int local_calc = a + b + c + d + e + f + g;
    
    printf("[compute_sum] In function. Arg1=%d, Arg6=%d, Arg7(from stack)=%d\n", a, f, g);
    printf("[compute_sum] Local var addresses: &local_temp=%p, &local_calc=%p\n", 
           (void*)&local_temp, (void*)&local_calc);

    return local_calc;
}

__attribute__((noinline))
void caller_function(void) {
    int x = 10;
    int y = 20;
    
    printf("[caller_function] Calling compute_sum with 7 arguments...\n");
    // Passing 7 arguments: 6 in registers, 1 on the stack
    int result = compute_sum(1, 2, 3, 4, 5, 6, 7);
    
    printf("[caller_function] Result returned: %d (x=%d, y=%d)\n", result, x, y);
}

int main(void) {
    printf("=== 0xLabs: x86_64 Stack Frame & Calling Convention Demo ===\n");
    caller_function();
    return 0;
}
