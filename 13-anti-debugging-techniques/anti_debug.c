#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <stdbool.h>
#include <stdint.h>
#include <x86intrin.h>

/*
 * ============================================================================
 * MODULE 13: Anti-Debugging Techniques & Reverse Engineering Analysis
 * ============================================================================
 * Software protections and DRM systems often employ anti-analysis checks
 * to detect if an analyst is attached with a debugger (GDB/LLDB).
 *
 * Techniques Demonstrated:
 *   1. PTRACE_TRACEME check (A process can only be traced by one debugger).
 *   2. /proc/self/status inspection (TracerPid field).
 *   3. RDTSC Timing Check (Detects hardware single-stepping delay).
 */

// 1. ptrace self-attach check
bool check_ptrace_attached(void) {
    // If GDB is already attached, ptrace(PTRACE_TRACEME) fails and returns -1
    if (ptrace(PTRACE_TRACEME, 0, 1, 0) < 0) {
        return true; // Debugger detected!
    }
    return false;
}

// 2. /proc/self/status TracerPid check
bool check_tracer_pid(void) {
    FILE *f = fopen("/proc/self/status", "r");
    if (!f) return false;

    char line[128];
    int tracer_pid = 0;

    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "TracerPid:", 10) == 0) {
            tracer_pid = atoi(line + 10);
            break;
        }
    }
    fclose(f);

    // If TracerPid > 0, an external process (GDB, strace) is attached
    return tracer_pid != 0;
}

// 3. RDTSC Cycle Count Timing Check
bool check_timing_delay(void) {
    uint64_t start = __rdtsc();

    // Small computation block
    volatile int dummy = 0;
    for (int i = 0; i < 100; i++) {
        dummy += i;
    }
    (void)dummy;

    uint64_t end = __rdtsc();
    uint64_t elapsed = end - start;

    // Normal execution takes < 5,000 cycles.
    // An analyst stepping through with GDB (stepi) takes millions of cycles!
    return elapsed > 100000ULL;
}

int main(void) {
    printf("=== 0xLabs: Anti-Debugging Protection Target ===\n");
    printf("[*] Running system diagnostics...\n");

    // Check 1: TracerPid in /proc/self/status (Check first before ptrace_traceme)
    if (check_tracer_pid()) {
        printf("[!] DETECTION: /proc/self/status TracerPid > 0! Debugger active.\n");
        return 1;
    }

    // Check 2: ptrace(PTRACE_TRACEME)
    if (check_ptrace_attached()) {
        printf("[!] DETECTION: ptrace(PTRACE_TRACEME) failed! Debugger active.\n");
        return 1;
    }

    // Check 3: RDTSC cycle timing check
    if (check_timing_delay()) {
        printf("[!] DETECTION: Execution latency anomaly (__rdtsc)! Single-stepping active.\n");
        return 1;
    }

    printf("[+] All checks passed. Protected application payload unlocked!\n");
    printf("[+] Secret Flag: 0xRE_CHALLENGE_SOLVED_2026\n");
    return 0;
}
