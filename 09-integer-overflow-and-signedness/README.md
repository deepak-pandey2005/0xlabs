# Module 09: Integer Overflow, Truncation & Signedness Mechanics

Integer vulnerabilities occur when arithmetic operations exceed the fixed bit-width of hardware storage registers, or when signed and unsigned types are implicitly cast.

While an integer overflow does not corrupt memory by itself, it frequently serves as the gateway to severe memory corruption (such as heap buffer overflows) by producing incorrect buffer allocation sizes or bypassing security bounds checks.

---

## 1. Hardware Representation & CPU Flags

In x86_64 CPU architecture:
- **Unsigned integers** represent values $[0, 2^N - 1]$. When exceeding $2^N - 1$, the hardware sets the **Carry Flag (CF)** in `EFLAGS` and wraps around to $0$.
- **Signed integers** use **Two's Complement** encoding to represent $[-2^{N-1}, 2^{N-1} - 1]$. When exceeding the maximum positive value, the hardware sets the **Overflow Flag (OF)** and flips the sign bit (`0x7FFFFFFF + 1 = 0x80000000 = -2147483648`).

---

## 2. Common Vulnerability Classes

### 2.1 The Undersized Allocation Vulnerability (Arithmetic Wrap)
A classic software security flaw occurs in dynamic allocation:
```c
uint32_t num_items = get_user_count(); // e.g. 1,073,741,825 (0x40000001)
uint32_t total_bytes = num_items * sizeof(uint32_t); // 0x40000001 * 4 = 0x100000004 -> wraps to 4 bytes!

void *buf = malloc(total_bytes); // Allocates a tiny 4-byte buffer

for (uint32_t i = 0; i < num_items; i++) {
    ((uint32_t *)buf)[i] = read_item(); // Massive Heap Buffer Overflow past 4 bytes!
}
```

### 2.2 Signedness Confusion & Implicit Sign Extension
In C, passing a negative signed integer to an API expecting `size_t` causes implicit casting:
```c
int user_len = -1;

if (user_len < MAX_ALLOWED) { // TRUE: -1 is less than 1024!
    memcpy(dest, src, user_len); // BUG: user_len is cast to (size_t)18446744073709551615ULL!
}
```
Because `memcpy` expects an unsigned 64-bit `size_t`, `-1` (`0xFFFFFFFFFFFFFFFF`) is interpreted as 18.4 exabytes, immediately triggering a memory segmentation fault.

---

## 3. Hands-on Demonstrations

### Exercise 1: Build and Run the Integer Lab
```bash
make
./integer_mechanics
```
Observe:
1. Hardware wrap-around and EFLAGS behavior.
2. Multiplication wrap: `1073741825 * 4 = 4 bytes`.
3. Signedness confusion: How `-1` bypasses `< 1024` but expands to $2^{64}-1$ bytes in `memcpy`.
4. Defensive validation using compiler intrinsics.

### Exercise 2: Catching Integer Bugs with UndefinedBehaviorSanitizer (UBSan)
```bash
./integer_mechanics_ubsan
```
UBSan automatically instruments code to detect and report undefined arithmetic at runtime.

---

## 4. Defensive Coding Practices

1. **Use Hardware-Checked Overflow Built-ins:**
   ```c
   size_t total_size;
   if (__builtin_mul_overflow(count, sizeof(type), &total_size)) {
       // Handle error securely: abort allocation
       return -1;
   }
   ```
2. **Always Use Unsigned Types for Sizes:**
   Use `size_t` or `uint64_t` for lengths and counts; never mix signed `int` with unsigned buffers.
3. **Compiler Hardening Flags:**
   - `-Wconversion -Wsign-conversion`: Flag implicit type casts.
   - `-fsanitize=signed-integer-overflow`: Trap signed overflows.
