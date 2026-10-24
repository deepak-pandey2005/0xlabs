# Module 03: Reverse Engineering Fundamentals

Reverse engineering is the process of analyzing a compiled binary to understand its internal components, control flow, and business logic without access to its original source code.

---

## 1. The Standard Reverse Engineering Methodology

Reverse engineering follows a structured triage and investigation process:

```
+-------------------------------------------------------------+
| 1. Initial Triage & File Identification                     |
|    - file <binary>, checksec, ldd                           |
+-------------------------------------------------------------+
                              |
                              v
+-------------------------------------------------------------+
| 2. Static Analysis: String & Symbol Extraction              |
|    - strings <binary>, readelf -s, nm                       |
+-------------------------------------------------------------+
                              |
                              v
+-------------------------------------------------------------+
| 3. Code Disassembly & Control Flow Reconstruction           |
|    - objdump -d -M intel <binary>                           |
|    - Identify main(), branches (cmp, test, jne, je)         |
+-------------------------------------------------------------+
                              |
                              v
+-------------------------------------------------------------+
| 4. Dynamic Analysis: Runtime Inspection with GDB           |
|    - Set breakpoints at verification checkpoints            |
|    - Inspect registers ($rax, $rdi) and comparison operands |
+-------------------------------------------------------------+
```

---

## 2. Common x86_64 Disassembly Patterns

When analyzing disassembly, recognize these canonical control flow patterns:

### Conditional Jumps
Condition flags are set by `cmp` (subtraction without storing result) or `test` (bitwise AND without storing result):
- `cmp eax, 0x0b` followed by `jne 0x401180`: Checks if `eax != 11` (e.g. length check `strlen != 11`).
- `test rax, rax` followed by `je 0x401190`: Checks if pointer or integer is `NULL` or `0`.

### Loops
Typically structured as:
```assembly
   mov    [rbp-0x8], 0          ; loop index i = 0
loop_start:
   cmp    [rbp-0x8], 10         ; compare i with loop limit
   jge    loop_end              ; break if i >= 10
   ...                          ; loop body
   add    [rbp-0x8], 1          ; i++
   jmp    loop_start            ; continue
loop_end:
```

---

## 3. The Challenge

You are provided with a compiled target binary: `challenge_auth`.
Your objective is to recover the valid key string required to produce:
```
[SUCCESS] Access Granted! Authentication Key Validated.
```

### Rules:
1. Do not read `challenge_auth.c`.
2. Perform all analysis strictly on `challenge_auth` or `challenge_auth_stripped` using `strings`, `objdump`, and `gdb`.

---

## 4. Hands-on Tasks

### Task 1: String Triage
```bash
make
strings challenge_auth | grep -E "SUCCESS|FAILED|Usage"
```
Notice what messages exist. Is the valid key stored as a plaintext string?

### Task 2: Disassembling `verify_key`
```bash
objdump -d -M intel challenge_auth | grep -A 35 "<verify_key>:"
```
Identify:
1. The length check (look for `cmp ..., 0xb` or 11).
2. The byte comparison loop.
3. The arithmetic instructions applied to each character (`xor`, `add`).

### Task 3: Inspecting in GDB
Set a breakpoint inside the loop in GDB:
```bash
gdb ./challenge_auth
(gdb) set disassembly-flavor intel
(gdb) break verify_key
(gdb) run AAAAAAAAAAA
(gdb) layout asm
(gdb) stepi
```

---

## Need Help?
Once you have attempted the analysis, consult [`solution_guide.md`](./solution_guide.md) for the complete walkthrough.
