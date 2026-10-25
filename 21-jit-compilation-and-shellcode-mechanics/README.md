# Module 21: JIT Code Generation & $W \oplus X$ Memory Protection

Just-In-Time (JIT) compilers—such as Google V8 (Chrome/Node.js), SpiderMonkey (Firefox), Java HotSpot VM, and .NET Core CLR—dynamically translate high-level code or bytecode into native CPU machine instructions in memory at runtime.

Because JIT engines create new executable code on the fly, they operate directly at the intersection of performance and hardware memory security: the **$W \oplus X$ (Write XOR Execute)** policy.

---

## 1. The $W \oplus X$ (DEP / Data Execution Prevention) Principle

The fundamental rule of modern platform memory security states:
$$\text{Memory Page Permissions} \in \{ \text{Writable}, \text{Executable} \}, \quad \text{Writable} \cap \text{Executable} = \emptyset$$

A memory page must **NEVER** be simultaneously Writable ($W$) and Executable ($X$):
- If memory is **Writable**, it must not be executable (prevents running injected code).
- If memory is **Executable**, it must be read-only (prevents tampering with existing code).

---

## 2. The JIT Compiler Memory Lifecycle

To produce and run code while respecting $W \oplus X$, JIT engines follow a strict four-stage state machine:

```
[ Stage 1: Allocation ]
  mmap(..., PROT_READ | PROT_WRITE)     --> Page is RW- (Writable, Not Executable)
           |
           v
[ Stage 2: Code Generation ]
  Emit x86_64 CPU Opcodes into buffer   --> lea eax, [rdi + rsi] ; ret
           |
           v
[ Stage 3: Hardening Transition ]
  mprotect(..., PROT_READ | PROT_EXEC)  --> Page transitioned to R-X (Executable, Read-Only!)
           |
           v
[ Stage 4: Dynamic Execution ]
  Cast pointer to function and call     --> int result = func(40, 2);
```

If an attacker or errant write attempts to modify the JIT code buffer during Stage 4, the CPU Memory Management Unit (MMU) catches the write to a read-only page and raises a hardware page fault exception (`SIGSEGV`).

---

## 3. Hands-on Demonstrations

### Exercise 1: Run the JIT Engine Demonstration
```bash
make
./run_jit_demo.sh
```

Follow the stages:
1. Allocates an anonymous memory page via `mmap(PROT_READ | PROT_WRITE)`.
2. Emits raw x86_64 opcodes:
   - `8d 04 37` $\implies$ `lea eax, [rdi + rsi]` (Adds first two integer arguments).
   - `c3` $\implies$ `ret`.
3. Revokes write permission and grants execute permission via `mprotect(PROT_READ | PROT_EXEC)`.
4. Executes the dynamically generated function: `add(40, 2) = 42`.
5. Attempts to write to the code page; observe the CPU MMU intercepting the violation with `SIGSEGV`.

---

## 4. Modern JIT Hardening Architectures

1. **Dual-Mapped Pages:**
   The JIT compiler maps the same physical memory frame to two distinct virtual addresses:
   - Address A: Read-Only + Executable ($R-X$) used for execution.
   - Address B: Read-Write ($RW-$) used exclusively by the compiler thread.
2. **Apple Hardened Runtime (PTHREAD_JIT_WRITE_PROTECT):**
   macOS/iOS uses hardware thread-level registers (`pthread_jit_write_protect_np`) to toggle write permissions on JIT memory per-thread, completely eliminating global writable mappings.
