# Module 05: Binary Mitigations & Defensive Systems

To counter memory corruption and stack-based vulnerabilities, operating systems, processor architectures, and compilers introduced multi-layered mitigation mechanisms.

Understanding these protections is critical for both vulnerability analysis and developing robust, secure software.

---

## 1. The Core Mitigations Matrix

| Mitigation | Primary Layer | Mechanism | Compiler Flags | Verification Command |
|---|---|---|---|---|
| **Stack Canary** | Compiler / Runtime | Inserts a pseudo-random guard value before `%rbp` and validates it before returning. | `-fstack-protector-strong` | `nm <bin> \| grep __stack_chk_fail` |
| **NX / DEP** | CPU / OS Kernel | Marks data pages (stack, heap) as non-executable via the page table NX bit ($W \oplus X$). | `-z noexecstack` (default) | `readelf -l <bin> \| grep -A1 GNU_STACK` |
| **PIE / ASLR** | Compiler / Kernel | Loads executable base and data structures at randomized virtual memory addresses on each launch. | `-fPIE -pie` | `readelf -h <bin> \| grep "Type: DYN"` |
| **Full RELRO** | Linker / Loader | Resolves all dynamic symbols at startup (`BIND_NOW`) and marks the GOT read-only. | `-Wl,-z,relro,-z,now` | `readelf -l <bin> \| grep GNU_RELRO` |
| **FORTIFY_SOURCE**| Compiler / Libc | Replaces unsafe standard functions (`strcpy`, `fgets`) with bounds-checked wrappers. | `-D_FORTIFY_SOURCE=2 -O2` | Compile-time warnings & runtime traps |

---

## 2. In-Depth Mechanism Analysis

### 2.1 Stack Canaries (Stack Protector)
When compiling with `-fstack-protector`:
1. **Prologue:** The compiler loads an 8-byte guard value from thread-local storage (`fs:0x28`) and pushes it onto the stack right before the saved `%rbp`:
   ```assembly
   mov    rax, QWORD PTR fs:0x28
   mov    QWORD PTR [rbp-0x8], rax
   ```
2. **Epilogue:** Before returning, the compiler XORs the stack value against `fs:0x28`:
   ```assembly
   mov    rax, QWORD PTR [rbp-0x8]
   sub    rax, QWORD PTR fs:0x28
   jne    __stack_chk_fail
   ```
3. If an overflow altered any byte of the canary, `sub` produces a non-zero value, jumping to `__stack_chk_fail()`, which immediately terminates the process (`*** stack smashing detected ***`).

### 2.2 Non-Executable Memory (NX / DEP)
The CPU memory management unit (MMU) uses the hardware **NX bit** (No-Execute) in the page table entries:
- Code resides in pages marked `Read + Execute` (`R-X`).
- Stack and heap memory are marked `Read + Write` (`RW-`).
- If `%rip` ever points to a stack or heap address, the CPU raises an immediate page fault exception (`SIGSEGV`), preventing execution of injected machine code.

### 2.3 Position-Independent Executables (PIE) & ASLR
Without PIE, the binary's `.text` section is fixed at a static base address (e.g., `0x400000`).
With `-fPIE -pie`:
- The binary is compiled as a shared object (`Type: DYN`).
- The Linux kernel's Address Space Layout Randomization (ASLR) assigns a randomized virtual base address each time the application is started.
- All code references use RIP-relative addressing (`lea rdi, [rip + 0x1234]`).

---

## 3. Hands-on Demonstrations

### Exercise 1: Run the Mitigation Auditor
```bash
make
./check_mitigations.sh
```
Compare the security profiles of:
1. `demo_no_canary`: All mitigations disabled.
2. `demo_with_canary`: Stack protector active; watch it abort when overflow occurs.
3. `demo_hardened`: Full defense-in-depth (Canary + NX + PIE + Full RELRO + FORTIFY).
4. `secure_refactoring`: Inherently safe code with bounded buffers.

### Exercise 2: Disassemble Canary Logic in GDB
Inspect how the compiler inserted canary instructions into `demo_with_canary`:
```bash
gdb -q ./demo_with_canary
(gdb) set disassembly-flavor intel
(gdb) disassemble test_canary
```
Locate the `fs:0x28` references in both the prologue and epilogue.

---

## 4. Defensive Coding Best Practices

Mitigations are safety nets; real security begins with sound software design:
1. **Always bound inputs:** Use `sizeof(dest)` instead of arbitrary limits.
2. **Avoid dangerous functions:** Ban `gets()`, unbounded `strcpy()`, `strcat()`, and `sprintf()`.
3. **Use compiler warnings as errors:** Build with `-Wall -Wextra -Werror -D_FORTIFY_SOURCE=2`.
4. **Prefer modern memory-safe languages** (Rust, Go) or modern standard C++ containers (`std::string`, `std::vector`, `std::span`) when possible.
