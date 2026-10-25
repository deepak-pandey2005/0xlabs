# Module 20: Binary Patch Diffing & Root Cause Analysis (RCA)

In vulnerability research and security intelligence, **Binary Patch Diffing** is the practice of comparing an unpatched binary ($v1.0$) against its security-updated successor ($v2.0$) to determine:
1. What exact code paths or basic blocks were altered.
2. What underlying vulnerability (CWE) was silently or explicitly fixed.
3. How to perform Root Cause Analysis (RCA) without vendor disclosure notes.

---

## 1. The Patch Diffing Methodology

```
+--------------------------+       +--------------------------+
| Version 1.0 (Vulnerable) |       |  Version 2.0 (Patched)   |
+--------------------------+       +--------------------------+
             |                                  |
             v                                  v
+--------------------------+       +--------------------------+
|  Disassembly Extraction  |       |  Disassembly Extraction  |
+--------------------------+       +--------------------------+
             \                                  /
              \                                /
               v                              v
           +--------------------------------------+
           | Binary Diff Engine (BinDiff/Diaphora)|
           |  - Match functions by call graph     |
           |  - Align basic block CFG edges       |
           |  - Highlight inserted instructions   |
           +--------------------------------------+
                              |
                              v
           +--------------------------------------+
           | Root Cause Analysis (RCA) Report     |
           |  - Classify bug (e.g. OOB write)     |
           |  - Identify missing validation check |
           +--------------------------------------+
```

---

## 2. Reading Assembly Diffs

When inspecting the assembly diff between `target_v1` and `target_v2`:

```diff
 xor    eax,eax
+cmp    DWORD PTR [rbp-0x6c],0x0        ; Added check: count <= 0?
+jle    <error_branch>                  ; If yes, branch to error
+cmp    DWORD PTR [rbp-0x6c],0x10       ; Added check: count > 16?
+jle    <loop_start>                    ; If in bounds, proceed
+mov    eax,0xffffffff                  ; Load -1 return value
+jmp    <epilogue>                      ; Abort
 mov    DWORD PTR [rbp-0x54],0x0        ; Original loop index init
```

### Analytical Deduction
1. **The Guard Variable:** `[rbp-0x6c]` is the second argument (`int count`) passed in `%esi`.
2. **The Constant `0x10`:** `0x10` is 16 decimal.
3. **The Conclusion:** Version 1.0 lacked upper and lower boundary validation on `count`. Because the local array holds 16 elements, supplying `count > 16` allowed an attacker to overwrite subsequent stack words.

---

## 3. Hands-on Demonstrations

### Exercise 1: Run the Binary Patch Differ
```bash
make
./diff_binaries.sh
```

Observe:
1. Automated disassembly extraction and normalization of `process_user_records`.
2. Unified assembly diff showing newly inserted guard basic blocks (`cmp`, `jle`, `mov eax, -1`).
3. Automated Root Cause Analysis classification.

---

## 4. Industry Standard Patch Diffing Tools

For full-scale reversing across gigabyte-scale binaries (e.g. Windows `ntoskrnl.exe`, Apple `mach_kernel`, or Chrome `libchrome.so`):
- **BinDiff (Google / Zynamics):** Graph-based binary differ integrating with Ghidra and IDA Pro.
- **Diaphora:** Advanced open-source program differ for IDA Pro.
- **Ghidra Version Tracking:** Native multi-version matching and symbol forward-porting tool.
