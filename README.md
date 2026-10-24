# 0xLabs: Low-Level Systems, Binary Analysis & Reverse Engineering Lab

A hands-on, educational laboratory for mastering x86_64 architecture, ELF binary structure, stack frame mechanics, memory corruption fundamentals, and modern platform mitigations.

---

## Curriculum Overview

This lab is structured into five progressive modules:

| Module | Topic | Core Concepts | Key Tools |
|---|---|---|---|
| **[01-binary-anatomy](./01-binary-anatomy/)** | Binary Structure & ELF | Sections (`.text`, `.data`, `.rodata`, `.bss`), headers, symbols | `readelf`, `objdump`, `nm` |
| **[02-x86_64-and-stack-frames](./02-x86_64-and-stack-frames/)** | Assembly & Stack Frames | x86_64 registers, System V AMD64 ABI, prologue/epilogue, stack growth | `gcc`, `objdump`, `gdb` |
| **[03-reverse-engineering-basics](./03-reverse-engineering-basics/)** | Reverse Engineering | Static analysis, control flow reconstruction, dynamic tracing, symbol recovery | `gdb`, `objdump`, `strings` |
| **[04-memory-corruption-mechanics](./04-memory-corruption-mechanics/)** | Memory Corruption Theory | Out-of-bounds writes, adjacent variable corruption, return address overwrites | `gcc`, `gdb` |
| **[05-binary-mitigations-and-defense](./05-binary-mitigations-and-defense/)** | Mitigations & Hardening | Stack Canaries, NX/DEP, PIE/ASLR, RELRO, safe C coding patterns | `gcc`, `readelf`, `gdb` |

---

## Quickstart

### Prerequisites Check
Ensure you have the standard GNU build and debugging toolchain installed:
```bash
gcc --version
gdb --version
objdump --version
readelf --version
make --version
```

### Build Everything
To compile all modules with their target compiler configurations:
```bash
make all
```

To clean all compiled binaries:
```bash
make clean
```

---

## Recommended Learning Flow

1. **Step 1:** Start with **`01-binary-anatomy`** to demystify how source code turns into an Executable and Linkable Format (ELF) file.
2. **Step 2:** Move to **`02-x86_64-and-stack-frames`** to learn how CPU registers (`%rax`, `%rdi`, `%rsp`, `%rbp`) manage function calls, local variables, and the stack.
3. **Step 3:** Complete **`03-reverse-engineering-basics`** to practice reading disassembly and reconstructing high-level program logic without source code.
4. **Step 4:** Study **`04-memory-corruption-mechanics`** in GDB to see how unbounded writes corrupt adjacent stack variables and control pointers.
5. **Step 5:** Finish with **`05-binary-mitigations-and-defense`** to understand how compilers and operating systems detect and mitigate memory corruption, and how to write secure code.
