# Module 04: Memory Corruption Mechanics

In low-level languages like C and C++, memory safety is not enforced by the compiler or runtime. Arrays are simply contiguous blocks of memory addresses without automatic boundary verification.

When an application accepts more data than an allocated buffer can hold, the surplus bytes spill over into adjacent memory on the stack.

---

## 1. Stack Memory Layout & Overwrite Mechanics

Consider the following function stack frame:

```
Address (Hex)      Content                                 Description
--------------------------------------------------------------------------------------
0x7fffffffd868     0x00000000004012ab                      Saved Return Address (%rip)
0x7fffffffd860     0x00007fffffffd870                      Saved Base Pointer (%rbp)
0x7fffffffd85c     0x00000000                              is_admin variable (4 bytes)
0x7fffffffd840     [ 'A', 'A', 'A', ... (16 bytes) ]       buffer[16]
```

### The Sequence of Overwrite
When writing sequentially from `buffer[0]` upwards in memory:
1. **Bytes 0 to 15:** Fill `buffer[16]`.
2. **Bytes 16 to 27:** Overwrite stack alignment/padding bytes.
3. **Bytes 28 to 31:** Overwrite the adjacent variable `is_admin`.
4. **Bytes 32 to 39:** Overwrite the Saved Base Pointer (`%rbp`).
5. **Bytes 40 to 47:** Overwrite the Saved Return Address (`%rip`).

When `check_access` finishes, the `ret` instruction pops whatever 8 bytes sit at the return address location into the `%rip` instruction pointer. If those bytes were overwritten with arbitrary data (e.g. `0x42424242`), the CPU attempts to execute instructions at an invalid or unmapped address, immediately raising a `SIGSEGV` (Segmentation Fault) hardware exception.

---

## 2. Hands-on Demonstrations

### Exercise 1: Run the Automated Analysis Script
```bash
make
./analyze_corruption.sh
```
Observe the three phases:
1. **Normal execution:** `is_admin` retains its original initialized value (`0`).
2. **Controlled Overwrite:** By providing 28 bytes of padding followed by `BBBB` (`0x42424242`), `is_admin` is modified in memory without ever assigning to it in code!
3. **GDB Hex Dump:** Step through the stack before and after the input is read to inspect the raw hex words.

### Exercise 2: Observing Instruction Pointer Corruption in GDB
To see how a return address overwrite leads to a crash:
```bash
# Provide 40 bytes of padding + 8 bytes of invalid return address (e.g., CCCCCCCC)
python3 -c 'import sys; sys.stdout.buffer.write(b"A"*40 + b"CCCCCCCC\n")' | ./vulnerable_buffer
```
The program will terminate with:
```
Segmentation fault (core dumped)
```

Now run it under GDB:
```bash
gdb ./vulnerable_buffer
(gdb) run < <(python3 -c 'import sys; sys.stdout.buffer.write(b"A"*40 + b"CCCCCCCC\n")')
```
When it crashes, check the instruction pointer:
```bash
(gdb) info registers rip
```
Notice that `rip` contains `0x4343434343434343` (ASCII `'CCCCCCCC'`), proving that the CPU attempted to return to the memory address supplied by the overflow.

---

## 3. Key Takeaways
- The lack of runtime bounds checking allows contiguous memory writes to breach variable and control flow boundaries.
- Corruption of adjacent data variables allows state hijacking without altering code.
- Corruption of saved stack frames disrupts program execution integrity.
- These risks led directly to the development of modern binary mitigations (explored in Module 05).
