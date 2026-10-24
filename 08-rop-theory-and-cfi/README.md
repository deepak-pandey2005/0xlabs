# Module 08: Return-Oriented Programming (ROP) Theory & Control Flow Integrity (CFI)

When operating systems introduced **NX / DEP (No-Execute / Data Execution Prevention)**, injecting custom machine code onto the stack became impossible because writable data pages are forbidden from executing.

To bypass this restriction, security researchers devised **Code-Reuse Attacks**: executing instructions that already exist in legitimate executable memory regions (`.text` or shared libraries like `libc`).

---

## 1. What is a "Gadget"?

A **gadget** is a short sequence of existing CPU instructions in an executable memory segment that culminates in a `ret` instruction (opcode `0xc3`):

```assembly
pop rdi
ret
```

### Why `ret` is Fundamental
The `ret` instruction performs two actions:
1. Pops the 8-byte value at the current top of the stack (`$rsp`) into the instruction pointer (`%rip`).
2. Increments the stack pointer (`$rsp = $rsp + 8`).

Because `ret` unconditionally transfers control to whatever address sits on top of the stack, a sequence of return addresses laid out in memory creates a **chain**:
- Gadget 1 runs -> executes `ret` -> CPU jumps to Gadget 2.
- Gadget 2 runs -> executes `ret` -> CPU jumps to Gadget 3.

In this paradigm, the **stack pointer (`%rsp`) effectively acts as the program counter**.

---

## 2. Anatomy of an x86_64 ROP Chain

In x86_64 Linux (System V AMD64 ABI), the first function argument must be in `%rdi`.

If an attacker wants to invoke `process_argument(0x1337)`:

```
Stack Memory (arranged from Low to High Addresses)
+--------------------------------------------------------+
| Address of 'pop rdi; ret' gadget                       | <-- Corrupted Return Address
+--------------------------------------------------------+
| 0x0000000000001337 (Value popped into %rdi by gadget)  |
+--------------------------------------------------------+
| Address of process_argument() function                 | <-- Called when gadget returns
+--------------------------------------------------------+
| Address of finalize_workflow() function                | <-- Called when process_argument returns
+--------------------------------------------------------+
```

### Execution Step-by-Step:
1. The vulnerable function finishes and executes `ret`.
2. `%rip` is loaded with `pop rdi; ret`.
3. The gadget executes `pop rdi`, loading `0x1337` into `%rdi`.
4. The gadget executes `ret`, popping `process_argument` into `%rip`.
5. `process_argument` executes with `%rdi = 0x1337`.
6. When `process_argument` returns, it pops `finalize_workflow` into `%rip`.

---

## 3. Modern Hardware & Compiler Mitigations

Because code-reuse relies on twisting legitimate instructions, modern defensive engineering operates at the CPU hardware and compiler level:

### 3.1 Intel CET (Control-flow Enforcement Technology)
1. **Shadow Stack:**
   - The CPU maintains a secondary, hardware-isolated stack in memory that cannot be modified by userland code.
   - When `call` executes, the return address is pushed to BOTH the normal stack and the shadow stack.
   - When `ret` executes, the CPU checks if the return address on the normal stack matches the shadow stack.
   - If they differ (indicating stack tampering or ROP), the CPU raises a Control Protection (`#CP`) hardware fault.
2. **Indirect Branch Tracking (IBT):**
   - Requires every indirect jump/call target to begin with the special instruction `endbr64` (End Branch 64).
   - If an indirect branch targets a gadget that does not begin with `endbr64`, the CPU halts immediately.

### 3.2 ARM Pointer Authentication (PAC)
- Cryptographically signs pointers (including return addresses) using secret keys stored in CPU registers and CPU tweak contexts.
- Before jumping, `autia` or `retaa` verifies the signature. Tampered pointers trigger an illegal address fault.

### 3.3 Clang Control Flow Integrity (CFI)
- Forward-edge CFI instrumented by compilers (e.g. `-fsanitize=cfi`).
- Analyzes function pointer types and virtual call tables at compile time, verifying at runtime that indirect function calls target only valid functions in the allowed call graph.

---

## 4. Hands-on Demonstrations

### Exercise 1: Finding Gadgets & CET Instructions
Run the automated scanner:
```bash
make
./find_gadgets.sh
```
1. Observe the symbol addresses for `print_banner`, `process_argument`, and `finalize_workflow`.
2. Observe how `objdump` identifies `pop ... ; ret` sequences.
3. Observe how the compiler automatically generated `endbr64` opcodes at function prologues for Intel CET compatibility.
