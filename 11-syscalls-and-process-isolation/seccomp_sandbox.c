#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <linux/seccomp.h>
#include <sys/syscall.h>

/*
 * ============================================================================
 * SECCOMP (Secure Computing Mode) Sandboxing
 * ============================================================================
 * SECCOMP_MODE_STRICT allows only 4 system calls:
 *   - read()
 *   - write()
 *   - _exit() (and exit_group)
 *   - sigreturn()
 * Any attempt to invoke any other system call (open, fork, execve, socket, etc.)
 * causes the Linux kernel to immediately kill the process with SIGKILL (or SIGSYS).
 */

int main(int argc, char *argv[]) {
    printf("=== 0xLabs: Kernel Seccomp Sandbox Demonstration ===\n");
    printf("[*] Process running in unconstrained mode. PID = %d\n", getpid());

    // Flush stdio streams before locking down syscalls
    fflush(stdout);

    printf("[*] Entering SECCOMP_MODE_STRICT sandbox...\n");
    fflush(stdout);

    // Lock down the process: no more execve, fork, socket, open!
    if (prctl(PR_SET_SECCOMP, SECCOMP_MODE_STRICT) != 0) {
        perror("prctl(PR_SET_SECCOMP)");
        return 1;
    }

    // Permitted: write(1, ...)
    const char ok_msg[] = "[+] Sandbox Active: sys_write is permitted.\n";
    syscall(SYS_write, 1, ok_msg, sizeof(ok_msg) - 1);

    if (argc > 1 && argv[1][0] == 'v') {
        const char attempt_msg[] = "[*] Attempting restricted syscall (getpid) inside strict sandbox...\n";
        syscall(SYS_write, 1, attempt_msg, sizeof(attempt_msg) - 1);

        // FORBIDDEN IN STRICT MODE: getpid (Syscall 39)
        // Kernel will instantly terminate this process!
        syscall(SYS_getpid);

        // Unreachable code:
        const char unreach[] = "[-] This line should never execute!\n";
        syscall(SYS_write, 1, unreach, sizeof(unreach) - 1);
    } else {
        const char exit_msg[] = "[+] Exiting cleanly using permitted exit syscall.\n";
        syscall(SYS_write, 1, exit_msg, sizeof(exit_msg) - 1);
        // Under SECCOMP_MODE_STRICT, standard libc return triggers prohibited cleanups.
        // Direct SYS_exit is one of the 4 permitted syscalls:
        syscall(SYS_exit, 0);
    }

    return 0;
}
