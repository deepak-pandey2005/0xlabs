#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>

/*
 * ============================================================================
 * MODULE 21: JIT Code Generation & W^X Memory Enforcement
 * ============================================================================
 * Concepts:
 *   1. Just-In-Time (JIT) Compilation: Dynamically generating and executing
 *      machine instructions at runtime (like V8, JVM, and PyPy).
 *   2. W^X (Write XOR Execute) Policy: Memory pages must NEVER be simultaneously
 *      Writable and Executable.
 *   3. The JIT Lifecycle:
 *      mmap(PROT_READ|PROT_WRITE) -> Emit opcodes -> mprotect(PROT_READ|PROT_EXEC) -> call.
 */

// Function pointer signature for JIT-generated function: int func(int x, int y);
typedef int (*jit_add_fn)(int, int);

void sigsegv_handler(int sig) {
    printf("\n[!] HARDWARE EXCEPTION INTERCEPTED: SIGSEGV (signal %d)\n", sig);
    printf("[✓] PROOF OF W^X: Writing to PROT_READ | PROT_EXEC memory was blocked by CPU MMU!\n");
    _exit(0);
}

int main(void) {
    printf("=== 0xLabs: JIT Code Generation & W^X Memory Enforcement ===\n\n");

    // Install signal handler to catch W^X violation cleanly
    signal(SIGSEGV, sigsegv_handler);

    size_t page_size = sysconf(_SC_PAGESIZE);
    printf("[*] System page size: %zu bytes\n", page_size);

    // Step 1: Allocate memory as READ + WRITE (No EXECUTE yet!)
    printf("[*] Step 1: Allocating page via mmap(PROT_READ | PROT_WRITE)...\n");
    uint8_t *code_page = (uint8_t *)mmap(
        NULL,
        page_size,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS,
        -1,
        0
    );

    if (code_page == MAP_FAILED) {
        perror("mmap");
        return 1;
    }
    printf("[+] Allocated JIT code buffer at: %p\n", (void *)code_page);

    // Step 2: Emit x86_64 machine code instructions into the buffer:
    // Implements: int add(int a, int b) { return a + b; }
    // Assembly:
    //   lea eax, [rdi + rsi]   ; 8d 04 37  (Add 1st arg in %edi and 2nd arg in %esi, put in %eax)
    //   ret                    ; c3
    const uint8_t opcodes[] = {
        0x8d, 0x04, 0x37,       // lea eax, [rdi + rsi]
        0xc3                    // ret
    };

    printf("[*] Step 2: Emitting %zu bytes of x86_64 machine instructions into page...\n", sizeof(opcodes));
    memcpy(code_page, opcodes, sizeof(opcodes));

    // Step 3: Transition memory permissions from RW to R-X (W^X transition)
    printf("[*] Step 3: Enforcing W^X: mprotect(PROT_READ | PROT_EXEC)...\n");
    if (mprotect(code_page, page_size, PROT_READ | PROT_EXEC) != 0) {
        perror("mprotect");
        return 1;
    }
    printf("[+] Memory is now EXECUTABLE and READ-ONLY (Write permission revoked).\n");

    // Step 4: Cast and execute dynamically generated function!
    printf("[*] Step 4: Executing JIT-generated machine code: add(40, 2)...\n");
    jit_add_fn dynamic_add = (jit_add_fn)code_page;
    int result = dynamic_add(40, 2);
    printf("[+] JIT Execution Result: %d (Calculated entirely from dynamic opcodes!)\n\n", result);

    // Step 5: Demonstrate W^X Violation (attempting to write to R-X page)
    printf("[*] Step 5: Testing W^X Security Boundary (attempting write to code page)...\n");
    fflush(stdout);

    // This write MUST trigger a hardware page fault exception (SIGSEGV)
    code_page[0] = 0x90; // NOP

    printf("[-] FAILURE: Write succeeded (W^X was NOT enforced)!\n");

    munmap(code_page, page_size);
    return 0;
}
