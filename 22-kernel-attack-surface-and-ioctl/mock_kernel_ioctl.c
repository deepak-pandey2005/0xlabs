/**
 * 0xLabs - Module 22: Linux Kernel Attack Surface & IOCTL Interface
 *
 * Demonstrates:
 * 1. The ioctl() interface architecture across Ring 3 (User) and Ring 0 (Kernel).
 * 2. User-pointer dereferences vs copy_from_user() / copy_to_user().
 * 3. Double-fetch vulnerability mechanics (TOCTOU in kernel memory copies).
 * 4. Struct alignment, bounds enforcement, and defensive kernel programming patterns.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>

#define KERNEL_BUF_CAPACITY 64

/* Simulation of Linux ioctl command macros */
#define IOCTL_MAGIC 'k'
#define IOCTL_CMD_READ_STATS   0x101
#define IOCTL_CMD_WRITE_DATA   0x102
#define IOCTL_CMD_RESET_DEVICE 0x103

/* User-Kernel communication struct */
typedef struct {
    uint32_t cmd;
    uint32_t data_len;
    void    *user_buffer;
} ioctl_request_t;

/* Simulated Kernel Device State */
typedef struct {
    uint8_t  kernel_buffer[KERNEL_BUF_CAPACITY];
    size_t   stored_bytes;
    uint32_t total_ops;
    uint32_t canary_sentinel; /* Detect kernel memory corruption */
} mock_kernel_device_t;

#define SENTINEL_VALUE 0xCAFEBABE

static mock_kernel_device_t g_device = {
    .stored_bytes = 0,
    .total_ops = 0,
    .canary_sentinel = SENTINEL_VALUE
};

/*
 * Simulated Kernel Functions:
 * In the real Linux kernel, user-space memory cannot be trusted.
 * Direct dereference of a user-space pointer in Ring 0 triggers SMAP
 * (Supervisor Mode Access Prevention) faults, or allows user-space
 * to page-fault the kernel arbitrarily.
 */

static int simulated_copy_from_user(void *kernel_dst, const void *user_src, size_t n) {
    if (!kernel_dst || !user_src) {
        return -EFAULT;
    }
    /* In real kernel, access_ok() verifies address range is in user space */
    memcpy(kernel_dst, user_src, n);
    return 0;
}

static int simulated_copy_to_user(void *user_dst, const void *kernel_src, size_t n) {
    if (!user_dst || !kernel_src) {
        return -EFAULT;
    }
    memcpy(user_dst, kernel_src, n);
    return 0;
}

/* Global flag to simulate another user thread modifying memory mid-syscall */
static bool g_simulate_race_flip = false;

/* =========================================================================
 * VULNERABLE KERNEL HANDLER 1: Direct User-Pointer Dereference
 * Violates user/kernel memory boundary isolation.
 * ========================================================================= */
int kernel_ioctl_vulnerable_direct_access(ioctl_request_t *user_req) {
    printf("[KERNEL] Executing VULNERABLE ioctl (Direct Pointer Dereference)...\n");
    g_device.total_ops++;

    if (user_req == NULL) {
        return -EFAULT;
    }

    uint32_t len = user_req->data_len;
    char *src = (char *)user_req->user_buffer;

    printf("[KERNEL] Direct read: len = %u, src ptr = %p\n", len, (void *)src);

    /* Direct memory read without copy_from_user() */
    for (uint32_t i = 0; i < len; i++) {
        g_device.kernel_buffer[i] = src[i];
    }
    g_device.stored_bytes = len;

    printf("[KERNEL] Direct copy finished. Bytes stored: %u\n", len);
    return 0;
}

/* =========================================================================
 * VULNERABLE KERNEL HANDLER 2: Double-Fetch Vulnerability
 *
 * In a double-fetch flaw, the kernel reads a value (e.g. data_len) from
 * user memory once to validate it, but then fetches it a second time
 * during the actual copy operation. A concurrent user-space thread can
 * flip the value between the check and the use!
 * ========================================================================= */
int kernel_ioctl_vulnerable_double_fetch(ioctl_request_t *user_req) {
    printf("[KERNEL] Executing VULNERABLE ioctl (Double-Fetch Check)...\n");
    g_device.total_ops++;

    /* --- FIRST FETCH: Validation Step --- */
    uint32_t check_len = 0;
    if (simulated_copy_from_user(&check_len, &user_req->data_len, sizeof(check_len)) != 0) {
        return -EFAULT;
    }

    printf("[KERNEL] Fetch 1 (Validation): user claims data_len = %u (Capacity: %d)\n",
           check_len, KERNEL_BUF_CAPACITY);

    if (check_len > KERNEL_BUF_CAPACITY) {
        printf("[KERNEL] Validation FAILED: %u > %d\n", check_len, KERNEL_BUF_CAPACITY);
        return -EINVAL;
    }
    printf("[KERNEL] Validation PASSED for length %u.\n", check_len);

    /*
     * SIMULATED RACE WINDOW:
     * In an SMP system with multiple CPU cores, an attacker's concurrent user-space
     * thread modifies `user_req->data_len` at this exact nanosecond!
     */
    if (g_simulate_race_flip) {
        printf("[USER RACE THREAD] *** Concurrent thread rewrote user_req->data_len to 120! ***\n");
        user_req->data_len = 120;
    }

    /* --- SECOND FETCH: Actual Copy Step (Re-reads from user space!) --- */
    ioctl_request_t req_fetch2;
    if (simulated_copy_from_user(&req_fetch2, user_req, sizeof(req_fetch2)) != 0) {
        return -EFAULT;
    }

    printf("[KERNEL] Fetch 2 (Operation): data_len re-read as %u!\n", req_fetch2.data_len);

    /*
     * FATAL BUG: The kernel uses the second fetched length without re-validation!
     * If user space modified it to be > KERNEL_BUF_CAPACITY, a kernel buffer
     * overflow occurs, corrupting adjacent kernel structures (canary sentinel).
     */
    if (simulated_copy_from_user(g_device.kernel_buffer, req_fetch2.user_buffer, req_fetch2.data_len) != 0) {
        return -EFAULT;
    }

    g_device.stored_bytes = req_fetch2.data_len;

    /* Verify if kernel sentinel was corrupted */
    if (g_device.canary_sentinel != SENTINEL_VALUE) {
        printf("[KERNEL BUG DETECTED] *** KERNEL MEMORY CORRUPTION ***\n");
        printf("    Kernel Canary: 0x%08X (Expected: 0x%08X)\n",
               g_device.canary_sentinel, SENTINEL_VALUE);
        printf("    Adjacent kernel structure smashed by out-of-bounds copy!\n");
    } else {
        printf("[KERNEL] Canary intact: 0x%08X\n", g_device.canary_sentinel);
    }

    return 0;
}

/* =========================================================================
 * SECURE KERNEL HANDLER: Single-Fetch & Atomic Snapshot Validation
 *
 * Pattern:
 * 1. Fetch the entire user request structure ONCE into a kernel stack frame.
 * 2. Validate all boundaries strictly against the immutable kernel-local copy.
 * 3. Never re-fetch validation fields from user-space memory.
 * ========================================================================= */
int kernel_ioctl_secure(ioctl_request_t *user_req) {
    printf("[KERNEL] Executing SECURE ioctl handler...\n");
    g_device.total_ops++;

    /* Step 1: Single Fetch of the request metadata */
    ioctl_request_t k_req;
    if (simulated_copy_from_user(&k_req, user_req, sizeof(k_req)) != 0) {
        return -EFAULT;
    }

    /* Step 2: Strict bounds check on the kernel stack copy (user cannot tamper) */
    if (k_req.data_len > KERNEL_BUF_CAPACITY) {
        printf("[KERNEL SECURE] Rejected: data_len (%u) exceeds buffer capacity (%d)\n",
               k_req.data_len, KERNEL_BUF_CAPACITY);
        return -EINVAL;
    }

    if (k_req.user_buffer == NULL && k_req.data_len > 0) {
        return -EFAULT;
    }

    /* Step 3: Copy payload using the verified, immutable kernel copy length */
    if (k_req.data_len > 0) {
        if (simulated_copy_from_user(g_device.kernel_buffer, k_req.user_buffer, k_req.data_len) != 0) {
            return -EFAULT;
        }
    }

    g_device.stored_bytes = k_req.data_len;
    printf("[KERNEL SECURE] Successfully copied %u bytes safely. Canary: 0x%08X\n",
           k_req.data_len, g_device.canary_sentinel);

    return 0;
}

/* =========================================================================
 * User-space Client / Test Harness
 * ========================================================================= */
int main(int argc, char **argv) {
    printf("=== 0xLabs: Kernel Attack Surface & IOCTL Interface Analysis ===\n\n");

    if (argc < 2) {
        printf("Usage: %s <mode>\n", argv[0]);
        printf("Modes:\n");
        printf("  --direct       Test direct user-pointer dereference flaw\n");
        printf("  --double-fetch Test double-fetch TOCTOU vulnerability\n");
        printf("  --secure       Test single-fetch immutable validation pattern\n");
        return 1;
    }

    if (strcmp(argv[1], "--direct") == 0) {
        char payload[] = "Userland Payload Directly Sent to Kernel Ring 0";
        ioctl_request_t req = {
            .cmd = IOCTL_CMD_WRITE_DATA,
            .data_len = strlen(payload) + 1,
            .user_buffer = payload
        };
        kernel_ioctl_vulnerable_direct_access(&req);

    } else if (strcmp(argv[1], "--double-fetch") == 0) {
        char large_payload[128];
        memset(large_payload, 'A', sizeof(large_payload));

        /* User-space request structure */
        ioctl_request_t req = {
            .cmd = IOCTL_CMD_WRITE_DATA,
            .data_len = 32, /* Initial benign size: passes check 1 (< 64) */
            .user_buffer = large_payload
        };

        printf("[USER] Preparing request with benign data_len = %u (passes check 1)\n", req.data_len);
        g_simulate_race_flip = true;

        kernel_ioctl_vulnerable_double_fetch(&req);

    } else if (strcmp(argv[1], "--secure") == 0) {
        char large_payload[128];
        memset(large_payload, 'B', sizeof(large_payload));

        ioctl_request_t req = {
            .cmd = IOCTL_CMD_WRITE_DATA,
            .data_len = 120, /* Attempting to send oversized payload */
            .user_buffer = large_payload
        };

        int ret = kernel_ioctl_secure(&req);
        if (ret != 0) {
            printf("[USER] Kernel correctly rejected dangerous request (ret = %d)\n", ret);
        }

        /* Sending valid payload */
        char valid_payload[] = "Valid Kernel Config";
        req.data_len = strlen(valid_payload) + 1;
        req.user_buffer = valid_payload;

        ret = kernel_ioctl_secure(&req);
        if (ret == 0) {
            printf("[USER] Valid request succeeded safely.\n");
        }

        /* Read stats using copy_to_user */
        uint32_t ops_readback = 0;
        simulated_copy_to_user(&ops_readback, &g_device.total_ops, sizeof(ops_readback));
        printf("[USER] Readback device total_ops via copy_to_user: %u\n", ops_readback);
    } else {
        fprintf(stderr, "Unknown mode: %s\n", argv[1]);
        return 1;
    }

    return 0;
}
