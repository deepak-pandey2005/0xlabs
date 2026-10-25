# 0xLabs: Low-Level Systems, Binary Analysis & Reverse Engineering Lab

A hands-on, educational laboratory for mastering x86_64 architecture, ELF binary structure, stack frame mechanics, memory corruption fundamentals, and modern platform mitigations.

---

## Curriculum Overview

This lab is structured into eleven progressive modules:

| Module | Topic | Core Concepts | Key Tools |
|---|---|---|---|
| **[01-binary-anatomy](./01-binary-anatomy/)** | Binary Structure & ELF | Sections (`.text`, `.data`, `.rodata`, `.bss`), headers, symbols | `readelf`, `objdump`, `nm` |
| **[02-x86_64-and-stack-frames](./02-x86_64-and-stack-frames/)** | Assembly & Stack Frames | x86_64 registers, System V AMD64 ABI, prologue/epilogue, stack growth | `gcc`, `objdump`, `gdb` |
| **[03-reverse-engineering-basics](./03-reverse-engineering-basics/)** | Reverse Engineering | Static analysis, control flow reconstruction, dynamic tracing, symbol recovery | `gdb`, `objdump`, `strings` |
| **[04-memory-corruption-mechanics](./04-memory-corruption-mechanics/)** | Memory Corruption Theory | Out-of-bounds writes, adjacent variable corruption, return address overwrites | `gcc`, `gdb` |
| **[05-binary-mitigations-and-defense](./05-binary-mitigations-and-defense/)** | Mitigations & Hardening | Stack Canaries, NX/DEP, PIE/ASLR, RELRO, safe C coding patterns | `gcc`, `readelf`, `gdb` |
| **[06-format-string-vulnerabilities](./06-format-string-vulnerabilities/)** | Format Strings & Info Leaks | Variadic calling convention, arbitrary stack reading, memory disclosure, `-Wformat-security` | `gcc`, `gdb` |
| **[07-heap-internals-and-lifecycle](./07-heap-internals-and-lifecycle/)** | Heap Internals & UAF | Glibc chunk metadata, tcache bin recycling, Use-After-Free, AddressSanitizer (ASan) | `gcc`, `gdb`, `libasan` |
| **[08-rop-theory-and-cfi](./08-rop-theory-and-cfi/)** | ROP & Control Flow Integrity | Code reuse theory, gadget chaining, Intel CET Shadow Stacks, IBT (`endbr64`), Clang CFI | `objdump`, `gcc` |
| **[09-integer-overflow-and-signedness](./09-integer-overflow-and-signedness/)** | Integer Mechanics & Wrap-around | Arithmetic wrap-around, signedness confusion, undersized allocations, UBSan | `gcc`, `ubsan` |
| **[10-dynamic-linking-and-hooking](./10-dynamic-linking-and-hooking/)** | Dynamic Linking & PLT/GOT | Runtime symbol resolution, `.plt`/`.got.plt`, function interposition via `LD_PRELOAD` | `gcc`, `readelf`, `ld.so` |
| **[11-syscalls-and-process-isolation](./11-syscalls-and-process-isolation/)** | Syscalls & Sandboxing | Ring 3 to Ring 0 transitions, raw `syscall` assembly, SECCOMP strict sandboxing | `gcc`, `prctl`, `seccomp` |

---

## Quickstart

### Prerequisites Check
Ensure you have the standard GNU build, debugging, and sanitizer toolchain installed:
```bash
gcc --version
gdb --version
objdump --version
readelf --version
make --version
```

### Build Everything
To compile all modules with their respective compiler configurations:
```bash
make all
```

To clean all compiled binaries across all modules:
```bash
make clean
```

---

## Recommended Learning Flow

1. **Step 1: [01-binary-anatomy](./01-binary-anatomy/)** — Demystify how compiled code is packaged into an ELF binary.
2. **Step 2: [02-x86_64-and-stack-frames](./02-x86_64-and-stack-frames/)** — Trace registers (`%rax`, `%rdi`, `%rsp`, `%rbp`) and stack frame construction.
3. **Step 3: [03-reverse-engineering-basics](./03-reverse-engineering-basics/)** — Reverse engineer a verification binary using disassembly and GDB.
4. **Step 4: [04-memory-corruption-mechanics](./04-memory-corruption-mechanics/)** — Inspect how adjacent stack variables are corrupted by out-of-bounds memory writes.
5. **Step 5: [05-binary-mitigations-and-defense](./05-binary-mitigations-and-defense/)** — Audit platform mitigations (Canaries, NX, PIE, Full RELRO, FORTIFY).
6. **Step 6: [06-format-string-vulnerabilities](./06-format-string-vulnerabilities/)** — Study variadic functions and stack memory disclosure bugs.
7. **Step 7: [07-heap-internals-and-lifecycle](./07-heap-internals-and-lifecycle/)** — Understand dynamic memory chunks, Use-After-Free (UAF), and AddressSanitizer.
8. **Step 8: [08-rop-theory-and-cfi](./08-rop-theory-and-cfi/)** — Explore code reuse concepts (gadgets) and modern hardware defenses (Intel CET, ARM PAC).
9. **Step 9: [09-integer-overflow-and-signedness](./09-integer-overflow-and-signedness/)** — Analyze arithmetic overflow, signedness bugs, and UBSan detection.
10. **Step 10: [10-dynamic-linking-and-hooking](./10-dynamic-linking-and-hooking/)** — Trace PLT/GOT resolution and intercept APIs via `LD_PRELOAD`.
11. **Step 11: [11-syscalls-and-process-isolation](./11-syscalls-and-process-isolation/)** — Examine direct x86_64 system calls and lock down execution with kernel SECCOMP sandboxing.
