# 0xLabs: Low-Level Systems, Binary Analysis & Reverse Engineering Lab

A hands-on, educational laboratory for mastering x86_64 architecture, ELF binary structure, stack frame mechanics, memory corruption fundamentals, and modern platform mitigations.

---

## Curriculum Overview

This lab is structured into eighteen progressive modules across fundamental theory and real-world implementations:

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
| **[12-coverage-guided-fuzzing](./12-coverage-guided-fuzzing/)** | Automated Bug Discovery | In-process fuzzing with LibFuzzer, coverage tracking, seed corpora, crash artifact triage | `clang`, `libFuzzer`, `ASan` |
| **[13-anti-debugging-techniques](./13-anti-debugging-techniques/)** | Anti-Debugging Analysis | PTRACE_TRACEME, TracerPid checks, RDTSC timing checks, GDB dynamic bypasses | `gcc`, `gdb`, `ptrace` |
| **[14-cpp-vtables-and-dynamic-dispatch](./14-cpp-vtables-and-dynamic-dispatch/)** | C++ vtables & Polymorphism | Itanium C++ ABI, `_vptr` layout, indirect calls, Forward-Edge CFI defenses | `g++`, `objdump`, `gdb` |
| **[15-control-flow-flattening-and-obfuscation](./15-control-flow-flattening-and-obfuscation/)** | Obfuscation & De-flattening | Control flow flattening, switch dispatchers, CFG recovery with GDB Python | `gcc`, `gdb`, `python` |
| **[16-real-world-case-study-heap-overflow](./16-real-world-case-study-heap-overflow/)** | Case Study: CVE-2021-3156 Pattern | Command-line argument unescaping, null-byte leaping, heap buffer overflow & patch diff | `gcc`, `ASan` |
| **[17-real-world-file-format-parser](./17-real-world-file-format-parser/)** | Real-World BMP Image Decoder | Binary header parsing (DIB), row stride calculations, multiplicative dimension wrap | `gcc`, `python`, `ASan` |
| **[18-race-conditions-and-toctou](./18-race-conditions-and-toctou/)** | Concurrency: TOCTOU Races | Time-of-check to time-of-use symlink attacks, atomic file operations (`O_NOFOLLOW`/`O_EXCL`) | `gcc`, `fcntl` |

---

## Quickstart

### Prerequisites Check
Ensure you have the standard GNU & LLVM build, debugging, and sanitizer toolchain installed:
```bash
gcc --version
g++ --version
clang --version
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
