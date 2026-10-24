# Module 02: x86_64 Architecture & Stack Frames

Every function call in compiled C or C++ creates an execution record in memory known as a **Stack Frame** (or Activation Record). Understanding the mechanics of the stack and CPU registers is essential for debugging, reverse engineering, and low-level systems programming.

---

## 1. System V AMD64 Calling Convention (Linux x86_64)

When a function is called on 64-bit Linux:

### Parameter Passing
1. First 6 integer/pointer arguments are passed via CPU registers:
   - **Arg 1:** `%rdi`
   - **Arg 2:** `%rsi`
   - **Arg 3:** `%rdx`
   - **Arg 4:** `%rcx`
   - **Arg 5:** `%r8`
   - **Arg 6:** `%r9`
2. Arguments 7 and beyond are pushed onto the stack in reverse order.
3. Return values are stored in `%rax` (or `%rdx:%rax` for 128-bit values).

### Stack Alignment Requirement
The stack pointer `%rsp` **must be 16-byte aligned** before any `call` instruction. The `call` instruction pushes an 8-byte return address, leaving `%rsp` 8-byte aligned at function entry.

---

## 2. Anatomy of a Stack Frame

The stack grows **downwards** from high memory addresses to low memory addresses:

```
Higher Addresses
+------------------------------------+
| 7th Argument (pushed by caller)    | [RBP + 16]
+------------------------------------+
| Return Address (pushed by 'call')  | [RBP + 8]  <-- Target RIP on 'ret'
+------------------------------------+
| Saved Old %rbp (pushed in prologue)| [RBP + 0]  <-- Current %rbp
+------------------------------------+
| Local Variable 1                   | [RBP - 4]
+------------------------------------+
| Local Variable 2                   | [RBP - 8]
+------------------------------------+
| Allocated Stack Space              | [RSP]      <-- Current %rsp
+------------------------------------+
Lower Addresses (Direction of Growth v)
```

---

## 3. Function Lifecycle in Assembly

### Function Prologue
When a function starts, it sets up its own stack frame:
```assembly
push rbp          ; Save caller's base pointer
mov  rbp, rsp     ; Set current frame pointer to top of stack
sub  rsp, 0x20    ; Reserve 32 bytes for local variables
```

### Function Epilogue
When a function finishes, it tears down its frame and returns:
```assembly
mov  rsp, rbp     ; Discard local variable space
pop  rbp          ; Restore caller's base pointer
ret               ; Pops the saved return address from stack into %rip
```
*(Note: Compilers often combine `mov rsp, rbp` and `pop rbp` into the single instruction `leave`).*

---

## 4. Hands-On Exercises

### Exercise 1: Compile and Inspect Assembly
```bash
make
cat stack_frame.s | grep -A 15 "compute_sum:"
```
Notice how the compiler moves `%edi`, `%esi`, `%edx`, `%ecx`, `%r8d`, and `%r9d` into the local stack frame, and accesses the 7th argument at `[rbp + 16]`.

### Exercise 2: Run the GDB Walkthrough
Execute the automated GDB walkthrough:
```bash
./run_gdb.sh
```
Follow the output to see:
1. The exact register contents on function entry.
2. The return address sitting on top of the stack (`$rsp`).
3. How `info symbol *(void**)$rsp` resolves directly back to `caller_function + offset`.
4. How `%rax` holds the function's return value when returning.

### Exercise 3: Manual Interactive Stepping with GDB
Try debugging interactively:
```bash
gdb ./stack_frame
(gdb) set disassembly-flavor intel
(gdb) break compute_sum
(gdb) run
(gdb) x/10gx $rsp
(gdb) stepi
(gdb) info registers
```
