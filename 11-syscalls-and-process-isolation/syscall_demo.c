#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>

/*
 * ============================================================================
 * MODULE 11: Linux System Calls & The Kernel Interface
 * ============================================================================
 * Concepts:
 *   - Syscall boundary: transitioning from Ring 3 (User) to Ring 0 (Kernel).
 *   - x86_64 register convention for syscall instruction:
 *       %rax = syscall number
 *       %rdi = arg 1, %rsi = arg 2, %rdx = arg 3, %r10 = arg 4, %r8 = arg 5, %r9 = arg 6
 *       %rcx and %r11 are clobbered by the CPU hardware during 'syscall'.
 *       Return value placed in %rax.
 */

// 1. Direct inline assembly implementation of sys_write (Syscall 1)
long raw_sys_write(int fd, const void *buf, size_t count) {
    long ret;
    __asm__ volatile (
        "mov $1, %%rax\n\t"        // Syscall 1 = sys_write
        "mov %1, %%edi\n\t"        // fd
        "mov %2, %%rsi\n\t"        // buf
        "mov %3, %%rdx\n\t"        // count
        "syscall\n\t"              // Hardware trap into kernel
        "mov %%rax, %0\n\t"        // Store return value
        : "=r" (ret)
        : "r" (fd), "r" (buf), "r" (count)
        : "rax", "rdi", "rsi", "rdx", "rcx", "r11", "memory"
    );
    return ret;
}

// 2. Direct inline assembly implementation of sys_getpid (Syscall 39)
long raw_sys_getpid(void) {
    long pid;
    __asm__ volatile (
        "mov $39, %%rax\n\t"       // Syscall 39 = sys_getpid
        "syscall\n\t"
        "mov %%rax, %0\n\t"
        : "=r" (pid)
        :
        : "rax", "rcx", "r11"
    );
    return pid;
}

int main(void) {
    const char msg[] = "[+] Hello from raw x86_64 syscall (sys_write without libc printf)!\n";
    raw_sys_write(1, msg, sizeof(msg) - 1);

    long pid = raw_sys_getpid();
    printf("[+] Direct sys_getpid returned PID: %ld\n", pid);

    return 0;
}
