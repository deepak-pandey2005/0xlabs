# Module 24: Uninitialized Memory, Struct Padding & Information Leaks

Welcome to Module 24 of **0xLabs**. This module explores **Information Disclosures (Info-Leaks)**—the prerequisite weapon for defeating ASLR (Address Space Layout Randomization) and KASLR (Kernel ASLR) in modern systems vulnerability research.

---

## 1. Why Info-Leaks are the Keystones of Modern Exploitation

With the industry-wide adoption of **W^X / DEP** and **ASLR**, attackers cannot execute injected shellcode or reliably jump to ROP gadgets because addresses change at every reboot and program invocation.

To mount any modern exploit, researchers must first uncover an **Info-Leak primitive**: a mechanism that discloses memory addresses (e.g. kernel text addresses, heap base addresses, stack pointers, or stack canaries).

---

## 2. The Uninitialized Struct Padding Pattern

C structures align their members according to hardware architecture requirements (typically natural alignment: 4-byte boundaries for 32-bit types, 8-byte boundaries for 64-bit types).

Consider:
```c
struct sensor_report {
    uint32_t id;      /* 4 bytes (offset 0..3) */
    /* PADDING HOLE: 4 bytes (offset 4..7) inserted by compiler */
    uint64_t value;   /* 8 bytes (offset 8..15) */
    uint16_t flag;    /* 2 bytes (offset 16..17) */
    /* PADDING HOLE: 6 bytes (offset 18..23) inserted by compiler */
};
```
Total declared fields: $4 + 8 + 2 = 14$ bytes.
Actual `sizeof(struct sensor_report)`: $24$ bytes ($10$ bytes of padding!).

### The Vulnerability:
When a kernel system call or driver handler declares:
```c
struct sensor_report rep;
rep.id = 1;
rep.value = 100;
rep.flag = 0;
copy_to_user(user_buf, &rep, sizeof(rep));
```
The compiler leaves the padding bytes unmodified. Whatever residual data was previously stored at those stack offsets (canaries, function return addresses into kernel `.text`, private crypto keys) is faithfully copied directly to user space!

Notable real-world instances:
- **CVE-2010-4073**: Linux kernel `ipc/compat.c` uninitialized padding leak.
- **CVE-2016-4470**: Linux kernel keyrings infoleak.
- **CVE-2018-14634**: Linux kernel integer overflow / padding disclosure.

---

## 3. Mitigations & Defenses

1. **Source Level**:
   Always zero out structs before filling fields:
   ```c
   struct sensor_report rep;
   memset(&rep, 0, sizeof(rep));
   /* OR C99 value initialization */
   struct sensor_report rep = {0};
   ```

2. **Compiler Mitigation**:
   Both GCC and Clang now provide automated compiler flags that zero out all local variables and stack frames automatically:
   ```bash
   -ftrivial-auto-var-init=zero
   ```
   This flag has been adopted in modern production Linux kernels (Kconfig `CONFIG_INIT_STACK_ALL_ZERO`) to completely eradicate stack information leaks.

---

## 4. Hands-On Verification

Run the test harness:
```bash
./test_infoleak.sh
```

Observe:
- `infoleak_lab` dumps the padding bytes showing the leaked stack canary (`0xDEADBEEF`).
- `infoleak_auto_zero` demonstrates that the compiler automatically eliminates the disclosure without changing any C code.
