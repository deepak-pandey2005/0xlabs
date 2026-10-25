#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdbool.h>

/*
 * ============================================================================
 * MODULE 18: Time-of-Check to Time-of-Use (TOCTOU) Race Conditions
 * ============================================================================
 * Concepts:
 *   1. Path-based checking vulnerability: access(path) followed by open(path).
 *   2. Symlink swap window of vulnerability.
 *   3. Modern Atomic Fix: open() with O_CREAT | O_EXCL | O_NOFOLLOW.
 */

// 1. VULNERABLE PATTERN: Separate check and use using file paths
int write_file_vulnerable(const char *path, const char *content, int delay_us) {
    printf("[*] [Vulnerable] Step 1: Checking permissions for '%s' with access()...\n", path);

    // TIME OF CHECK (TOC)
    // Check if file is writable or can be created in directory
    if (access(path, F_OK) == 0 && access(path, W_OK) != 0) {
        fprintf(stderr, "[-] Access check failed: not writable.\n");
        return -1;
    }

    printf("[+] Access check passed.\n");

    // ARTIFICIAL RACE WINDOW (Simulating preemption or thread delay)
    if (delay_us > 0) {
        printf("[*] [Race Window Active] Sleeping %d microseconds...\n", delay_us);
        usleep(delay_us);
    }

    // TIME OF USE (TOU)
    // Opens by pathname; follows symlinks if an attacker swapped the path during the window!
    FILE *f = fopen(path, "w");
    if (!f) {
        perror("[-] fopen failed");
        return -1;
    }

    printf("[+] [Vulnerable] Step 2: Writing data to '%s'...\n", path);
    fputs(content, f);
    fclose(f);
    return 0;
}

// 2. SECURE PATTERN: Atomic open with O_NOFOLLOW | O_CREAT | O_EXCL
int write_file_secure(const char *path, const char *content) {
    printf("[*] [Secure] Opening file atomically with O_NOFOLLOW | O_CREAT | O_EXCL...\n");

    // ATOMIC OPERATION:
    // O_NOFOLLOW: Refuses to open if path is a symbolic link!
    // O_EXCL: Fails if file already exists (guarantees new file creation without race)!
    int fd = open(path, O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW, 0600);
    if (fd < 0) {
        if (errno == EEXIST) {
            fprintf(stderr, "[!] DEFENSIVE INTERCEPTION: File already exists or is a symlink (EEXIST)!\n");
        } else if (errno == ELOOP) {
            fprintf(stderr, "[!] DEFENSIVE INTERCEPTION: Symlink detected via O_NOFOLLOW (ELOOP)!\n");
        } else {
            perror("[-] open failed");
        }
        return -1;
    }

    // Safe writing via descriptor (cannot be swapped out by filesystem race)
    write(fd, content, strlen(content));
    close(fd);

    printf("[+] [Secure] Data written safely to new file descriptor.\n");
    return 0;
}

int main(int argc, char *argv[]) {
    printf("=== 0xLabs: TOCTOU File Race Condition Demonstration ===\n\n");

    if (argc < 3) {
        fprintf(stderr, "Usage: %s <--vuln|--secure> <file_path> [delay_us]\n", argv[0]);
        return 1;
    }

    bool secure = (strcmp(argv[1], "--secure") == 0);
    const char *path = argv[2];
    int delay = (argc > 3) ? atoi(argv[3]) : 0;
    const char *data = "CRITICAL_PAYLOAD_DATA\n";

    if (secure) {
        return write_file_secure(path, data);
    } else {
        return write_file_vulnerable(path, data, delay);
    }
}
