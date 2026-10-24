#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/*
 * ============================================================================
 * MODULE 07: Heap Internals, Chunk Layout & Lifecycle Management
 * ============================================================================
 * Concepts:
 *   1. Heap chunk headers: size field and PREV_INUSE bit (0x1).
 *   2. Use-After-Free (UAF) mechanics: dangling pointers pointing to recycled chunks.
 *   3. Double-Free detection in modern glibc.
 *   4. Safe memory lifecycle management (pointer zeroing).
 */

typedef struct Session {
    int id;
    int role; // 0 = standard user, 1 = administrator
    char username[24];
} Session_t;

typedef struct Note {
    char text[32];
} Note_t;

void inspect_chunk_header(void *user_ptr) {
    // In 64-bit glibc, chunk header precedes user payload by 8 or 16 bytes:
    // [prev_size (8 bytes)] [size + flags (8 bytes)] [user payload ...]
    uint64_t *header = (uint64_t *)((char *)user_ptr - 8);
    uint64_t size_and_flags = *header;
    uint64_t actual_size = size_and_flags & ~0x7;
    int prev_inuse = size_and_flags & 0x1;

    printf("[*] Inspecting chunk for user ptr %p:\n", user_ptr);
    printf("    Header address       : %p\n", (void *)header);
    printf("    Raw size + flags word: 0x%016lx\n", size_and_flags);
    printf("    Chunk allocation size: %lu bytes\n", actual_size);
    printf("    PREV_INUSE flag      : %d\n\n", prev_inuse);
}

void demo_use_after_free(void) {
    printf("=== Demonstration 1: Use-After-Free (UAF) Mechanics ===\n");

    // 1. Allocate a session object
    Session_t *admin_session = (Session_t *)malloc(sizeof(Session_t));
    admin_session->id = 1;
    admin_session->role = 1; // Admin role
    strncpy(admin_session->username, "superadmin", sizeof(admin_session->username));
    
    printf("[+] Allocated admin_session at %p (role = %d, username = %s)\n",
           (void *)admin_session, admin_session->role, admin_session->username);
#ifndef __SANITIZE_ADDRESS__
    inspect_chunk_header(admin_session);
#endif

    // 2. Free session, but leave dangling pointer (DO NOT NULL IT)
    printf("[*] Freeing admin_session...\n");
    free(admin_session);
    printf("[!] admin_session pointer is now DANGLING at %p\n", (void *)admin_session);

    // 3. Allocate a new Note object of identical size (glibc recycles the chunk from tcache)
    Note_t *user_note = (Note_t *)malloc(sizeof(Note_t));
    memset(user_note->text, 'A', sizeof(user_note->text) - 1);
    user_note->text[sizeof(user_note->text) - 1] = '\0';
    
    printf("[+] Allocated user_note at %p (text = %s)\n", (void *)user_note, user_note->text);

    // 4. Observe that glibc reused the EXACT same chunk memory address!
    if ((void *)admin_session == (void *)user_note) {
        printf("[!] CHUNK RECYCLED: admin_session and user_note share the identical address!\n");
    }

    // 5. Dereference the dangling pointer (UAF reading corrupted/reused data)
    printf("[*] Reading admin_session through dangling pointer:\n");
    printf("    admin_session->id   : 0x%08x\n", admin_session->id);
    printf("    admin_session->role : 0x%08x (Corrupted by Note payload!)\n", admin_session->role);
    printf("    admin_session->name : %s\n\n", admin_session->username);

    free(user_note);
}

void demo_safe_lifecycle(void) {
    printf("=== Demonstration 2: Safe Memory Lifecycle Pattern ===\n");

    Session_t *session = (Session_t *)malloc(sizeof(Session_t));
    session->id = 2;
    session->role = 0;
    strncpy(session->username, "guest", sizeof(session->username));

    printf("[+] Allocated session at %p\n", (void *)session);

    // Defensive free pattern: free and immediately zero pointer
    free(session);
    session = NULL;
    printf("[+] Safely freed and set session = NULL.\n");

    // Any subsequent access causes predictable, safe NULL-pointer trap rather than silent memory corruption
    if (session == NULL) {
        printf("[+] Defensive check: session is NULL, safely preventing Use-After-Free dereference.\n\n");
    }
}

int main(void) {
    printf("=== 0xLabs: Heap Internals & Lifecycle Management ===\n\n");
    demo_use_after_free();
    demo_safe_lifecycle();
    return 0;
}
