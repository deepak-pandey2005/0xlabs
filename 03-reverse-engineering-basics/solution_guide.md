# Module 03: Reverse Engineering Solution Guide

This guide walks through both static and dynamic analysis approaches to recover the secret key from `challenge_auth`.

---

## Approach 1: Static Disassembly Analysis

### Step 1: Disassemble `verify_key`
Run:
```bash
objdump -d -M intel challenge_auth
```

Locate the `verify_key` function:
1. **Length Check:**
   Notice:
   ```assembly
   call   strlen
   cmp    rax, 0xb
   je     ...
   mov    eax, 0x0
   ret
   ```
   This confirms the input must be exactly `0xb` (11 decimal) characters long.

2. **Loop and Transformation:**
   Inside the verification loop:
   ```assembly
   movzx  eax, BYTE PTR [rbp+rax*1-...] ; Load input[i]
   mov    edx, DWORD PTR [rbp-0x4]      ; Load loop index i
   add    edx, 0x5a                    ; Add 0x5A to i
   xor    eax, edx                     ; transformed = input[i] ^ (0x5A + i)
   ```
   Each character at position `i` is XORed with `(0x5A + i)`.

3. **Comparison Array:**
   The transformed byte is compared against an array stored in the `.rodata` section:
   ```assembly
   lea    rdx, [rip+0x... <EXPECTED_TRANSFORM>]
   cmp    al, BYTE PTR [rdx+rax*1]
   ```

### Step 2: Read the Comparison Bytes from Binary
Use `readelf` or `objdump` to dump the `.rodata` bytes of `EXPECTED_TRANSFORM`:
```bash
objdump -s -j .rodata challenge_auth
```
We observe the target bytes:
`6a 23 10 1c 1c 0c 4d 53 52 51 52`

### Step 3: Reversing the XOR Operation
Since XOR is symmetric (`A ^ B = C <=> C ^ B = A`):
`original_char[i] = target_byte[i] ^ (0x5A + i)`

Reconstruct with a one-liner:
```bash
python3 -c '
targets = [0x6a, 0x23, 0x10, 0x1c, 0x1c, 0x0c, 0x4d, 0x53, 0x52, 0x51, 0x52]
flag = "".join(chr(val ^ (0x5A + i)) for i, val in enumerate(targets))
print(f"Decoded Key: {flag}")
'
```
Output:
```
Decoded Key: 0xLABS-2026
```

---

## Approach 2: Dynamic Analysis with GDB

Instead of manual decoding, trace the comparison directly in GDB:
```bash
gdb ./challenge_auth
(gdb) set disassembly-flavor intel
(gdb) break verify_key
(gdb) run 12345678901
```
Set a breakpoint at the `cmp al, BYTE PTR [rdx]` instruction:
```bash
(gdb) layout asm
(gdb) x/11xb $rdx
```
Examining `$rdx` immediately reveals the entire array of expected transformed bytes right in memory.

---

## Verification

Test the decoded key against the binary:
```bash
./challenge_auth 0xLABS-2026
```
Result:
```
=== 0xLabs: Reverse Engineering Challenge ===
[SUCCESS] Access Granted! Authentication Key Validated.
```
