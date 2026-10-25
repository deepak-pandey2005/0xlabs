/**
 * 0xLabs - Module 25: Linux Kernel Heap Mechanics & The SLUB Allocator
 *
 * Demonstrates:
 * 1. Linux Kernel SLUB allocator architecture (kmem_cache, size bins, freelist).
 * 2. Freelist pointer storage and CONFIG_SLAB_FREELIST_HARDENED (cookie XOR).
 * 3. Kernel Use-After-Free (UAF) and object slot cross-type recycling.
 * 4. Privilege escalation via stale object pointer reinterpretation.
 * 5. Defensive mechanisms: kref reference counting, pointer nullification, and sanitization.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

#define SLAB_OBJECT_SIZE 64
#define SLAB_MAX_OBJECTS 8
#define RANDOM_SLAB_COOKIE 0x51A8C00C1E55AA33ULL

/* =========================================================================
 * Simulated Linux Kernel SLUB Cache Infrastructure
 * ========================================================================= */

typedef struct kmem_cache {
    const char *name;
    size_t      object_size;
    uint64_t    random_cookie;
    void       *freelist;
    uint8_t     memory_pool[SLAB_MAX_OBJECTS * SLAB_OBJECT_SIZE];
    bool        hardened;
} kmem_cache_t;

/* Freelist encoding helper (CONFIG_SLAB_FREELIST_HARDENED) */
static inline void *freelist_ptr_encode(const kmem_cache_t *cache, void *ptr, void *ptr_addr) {
    if (!cache->hardened || ptr == NULL) {
        return ptr;
    }
    uint64_t encoded = (uint64_t)ptr ^ cache->random_cookie ^ (uint64_t)ptr_addr;
    return (void *)encoded;
}

static inline void *freelist_ptr_decode(const kmem_cache_t *cache, void *encoded_ptr, void *ptr_addr) {
    if (!cache->hardened || encoded_ptr == NULL) {
        return encoded_ptr;
    }
    uint64_t decoded = (uint64_t)encoded_ptr ^ cache->random_cookie ^ (uint64_t)ptr_addr;
    return (void *)decoded;
}

/* Initialize a slab cache */
void kmem_cache_init(kmem_cache_t *cache, const char *name, bool hardened) {
    cache->name = name;
    cache->object_size = SLAB_OBJECT_SIZE;
    cache->random_cookie = RANDOM_SLAB_COOKIE;
    cache->hardened = hardened;
    cache->freelist = NULL;

    /* Build initial singly-linked freelist */
    for (int i = SLAB_MAX_OBJECTS - 1; i >= 0; i--) {
        void *obj = (void *)&cache->memory_pool[i * SLAB_OBJECT_SIZE];
        void **next_ptr_slot = (void **)obj;

        *next_ptr_slot = freelist_ptr_encode(cache, cache->freelist, obj);
        cache->freelist = obj;
    }
}

/* kmem_cache_alloc: Pop head of freelist */
void *kmem_cache_alloc(kmem_cache_t *cache) {
    if (cache->freelist == NULL) {
        printf("[KERNEL SLUB] Cache '%s' exhausted (Out of Memory)!\n", cache->name);
        return NULL;
    }

    void *obj = cache->freelist;
    void **next_ptr_slot = (void **)obj;
    void *raw_next = *next_ptr_slot;

    cache->freelist = freelist_ptr_decode(cache, raw_next, obj);
    memset(obj, 0, cache->object_size);
    return obj;
}

/* kmem_cache_free: Push to head of freelist */
void kmem_cache_free(kmem_cache_t *cache, void *obj) {
    if (obj == NULL) return;

    void **next_ptr_slot = (void **)obj;
    *next_ptr_slot = freelist_ptr_encode(cache, cache->freelist, obj);
    cache->freelist = obj;
}

/* =========================================================================
 * Kernel Structures (Both fit in the 64-byte SLUB bin)
 * ========================================================================= */

// Structure 1: Regular Unprivileged Session Object
typedef struct {
    uint32_t session_id;
    char     username[32];
    uint32_t is_authenticated;
    uint8_t  padding[24]; // Pad to 64 bytes
} user_session_t;

// Structure 2: Privileged Kernel Credentials Object
typedef struct {
    uint32_t uid;              // 0 = root, 1000 = normal user
    uint32_t gid;
    uint64_t capabilities;     // 0xFFFFFFFFFFFFFFFF = full root caps
    char     security_tag[32];
    uint8_t  padding[16];      // Pad to 64 bytes
} kernel_credential_t;

/* =========================================================================
 * Demonstrations
 * ========================================================================= */

static kmem_cache_t g_kmalloc_64;
static user_session_t *g_dangling_session = NULL;

void demo_vulnerable_kernel_uaf(void) {
    printf("[*] Initializing standard kmalloc-64 SLUB cache...\n");
    kmem_cache_init(&g_kmalloc_64, "kmalloc-64", false);

    printf("\n[1] Allocating user_session_t for unprivileged user 'alice'...\n");
    user_session_t *session = (user_session_t *)kmem_cache_alloc(&g_kmalloc_64);
    session->session_id = 42;
    strncpy(session->username, "alice", sizeof(session->username) - 1);
    session->is_authenticated = 1;

    printf("    Session allocated at: %p\n", (void *)session);
    printf("    Username: %s, Auth: %u\n", session->username, session->is_authenticated);

    // Save a reference to simulate a kernel driver retaining a stale pointer
    g_dangling_session = session;

    printf("\n[2] User logs out -> Kernel frees user_session_t...\n");
    // BUG: Driver frees the memory but DOES NOT NULLIFY g_dangling_session!
    kmem_cache_free(&g_kmalloc_64, session);
    printf("    Freed slot %p back to SLUB freelist.\n", (void *)session);
    printf("    [!] Vulnerability: g_dangling_session STILL points to %p!\n",
           (void *)g_dangling_session);

    printf("\n[3] System daemon creates privileged kernel_credential_t...\n");
    // SLUB allocator reclaims the most recently freed slot from the freelist (LIFO)
    kernel_credential_t *root_cred = (kernel_credential_t *)kmem_cache_alloc(&g_kmalloc_64);
    root_cred->uid = 0; // Root UID
    root_cred->gid = 0;
    root_cred->capabilities = 0xFFFFFFFFFFFFFFFFULL;
    strncpy(root_cred->security_tag, "SYSTEM_SECURITY_ADMIN", sizeof(root_cred->security_tag) - 1);

    printf("    Credential allocated at: %p (SAME SLAB SLOT!)\n", (void *)root_cred);
    printf("    Credential UID: %u, Caps: 0x%016lX\n", root_cred->uid, root_cred->capabilities);

    printf("\n[4] Attacker accesses stale g_dangling_session pointer...\n");
    printf("    Interpreting memory through dangling user_session_t:\n");
    printf("    Stale session_id (maps to root_cred->uid): %u\n", g_dangling_session->session_id);
    printf("    Stale username (maps to root_cred->capabilities/tag): %s\n", g_dangling_session->username);

    printf("\n[5] Attacker writes via dangling pointer to escalate privileges:\n");
    printf("    Overwriting session_id to 9999...\n");
    g_dangling_session->session_id = 9999;

    printf("\n[6] Checking root_cred structure after corruption:\n");
    printf("    root_cred->uid is now: %u (CORRUPTED VIA UAF!)\n", root_cred->uid);
    if (root_cred->uid == 9999) {
        printf("    [CRITICAL] Kernel credential modified via Use-After-Free!\n");
    }
}

void demo_hardened_freelist(void) {
    printf("\n============================================================\n");
    printf("[*] Demonstrating CONFIG_SLAB_FREELIST_HARDENED...\n");
    kmem_cache_init(&g_kmalloc_64, "kmalloc-64-hardened", true);

    void *obj1 = kmem_cache_alloc(&g_kmalloc_64);
    void *obj2 = kmem_cache_alloc(&g_kmalloc_64);
    printf("    Allocated obj1: %p, obj2: %p\n", obj1, obj2);

    kmem_cache_free(&g_kmalloc_64, obj1);
    printf("    Freed obj1 back to cache.\n");

    // Inspect the freelist pointer inside obj1
    uint64_t encoded_ptr = *(uint64_t *)obj1;
    printf("    Raw pointer stored inside freed obj1: 0x%016lX\n", encoded_ptr);
    printf("    Expected next address:               %p\n", g_kmalloc_64.freelist);
    printf("    Notice: The pointer is heavily obfuscated with (ptr ^ cookie ^ addr)!\n");
    printf("    An attacker attempting to overwrite the freelist pointer without\n");
    printf("    knowing the random kernel cookie will fail pointer decoding.\n");

    kmem_cache_free(&g_kmalloc_64, obj2);
}

int main(int argc, char **argv) {
    printf("=== 0xLabs - Module 25: Linux Kernel SLUB Allocator & Kernel UAF ===\n\n");

    bool run_uaf = true;
    bool run_hardening = true;

    if (argc > 1) {
        if (strcmp(argv[1], "--uaf") == 0) run_hardening = false;
        else if (strcmp(argv[1], "--hardened") == 0) run_uaf = false;
    }

    if (run_uaf) {
        demo_vulnerable_kernel_uaf();
    }

    if (run_hardening) {
        demo_hardened_freelist();
    }

    printf("\n============================================================\n");
    printf("[✓] Kernel SLUB Lab Execution Completed!\n");
    return 0;
}
