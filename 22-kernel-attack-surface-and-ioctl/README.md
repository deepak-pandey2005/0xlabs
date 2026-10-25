# Module 22: Linux Kernel Attack Surface & The IOCTL Interface

Welcome to Module 22 of **0xLabs**. This module focuses on operating system kernel vulnerability research, exploring how user-space (Ring 3) communicates with kernel-space (Ring 0) and why device driver control functions (`ioctl`) are the most common source of local privilege escalation (LPE) vulnerabilities.

---

## 1. The Kernel Attack Surface

In a typical operating system, the kernel protects hardware and critical data structures by running in Ring 0 (privileged mode), while user applications run in Ring 3 (unprivileged mode).

The entry points from Ring 3 into Ring 0 include:
1. **System Calls (`syscall`)**: Fixed POSIX interfaces (`read`, `write`, `open`, `mmap`).
2. **Virtual Filesystems (`sysfs`, `procfs`)**: Reading/writing `/sys` or `/proc` pseudo-files.
3. **Character/Block Device Drivers (`/dev/*`)**: Controlled via the generic `ioctl` (Input/Output Control) system call.

Because generic system calls are heavily scrutinized, device drivers—often written by third-party hardware vendors—are the primary target for kernel vulnerability research.

---

## 2. Core Concepts & Pitfalls

### Pitfall 1: Direct User-Pointer Dereferencing & SMAP
A kernel driver cannot simply do:
```c
// DANGEROUS IN RING 0:
uint32_t len = user_req->len;
char *data = user_req->buffer;
```
- **SMAP (Supervisor Mode Access Prevention)**: Modern x86_64 CPUs enforce hardware page protections preventing the kernel from accessing user-space pages unless explicitly enabled via `stac` (Set AC Flag in EFLAGS). Direct dereference triggers a kernel panic / page fault.
- **`copy_from_user()` / `copy_to_user()`**: The kernel must use safe copy primitives that verify address ranges (`access_ok`) and catch page faults cleanly via the kernel exception table (`fixup`).

### Pitfall 2: The Double-Fetch Vulnerability (Kernel TOCTOU)
A double-fetch occurs when the kernel reads a piece of user memory twice:
1. **Fetch 1 (Validation)**:
   ```c
   copy_from_user(&len, &user_req->len, sizeof(len));
   if (len > KERNEL_BUF_MAX) return -EINVAL; // Bounds check passed!
   ```
2. **Fetch 2 (Execution)**:
   ```c
   // The driver copies the entire request structure or re-reads len:
   copy_from_user(&req, user_req, sizeof(req));
   copy_from_user(kernel_buf, req.user_buffer, req.len); // USES RE-FETCHED LEN!
   ```

Because user-space memory is shared among all threads of the calling process, an attacker thread running on another CPU core can alter `user_req->len` between Fetch 1 and Fetch 2. If timed correctly, the kernel passes validation with a small size, but executes the copy with an enormous size, causing a kernel heap or stack buffer overflow!

---

## 3. The Defensive Pattern: Single-Fetch Snapshot

To eliminate double-fetch vulnerabilities:
1. **Single Fetch**: Copy the user request struct into kernel-space memory (stack or allocated heap chunk) once at the beginning of the handler.
2. **Immutable Validation**: Perform all checks against the local kernel copy.
3. **Execution**: Use the validated local variables to perform the operation. User space can no longer modify the values under the kernel's feet.

---

## 4. Hands-On Lab

Run the automated test harness:
```bash
./test_ioctl.sh
```

Inspect how:
1. `--direct` demonstrates the hazards of ignoring user/kernel boundaries.
2. `--double-fetch` demonstrates kernel memory corruption where an adjacent sentinel (`0xCAFEBABE`) is smashed by an out-of-bounds write.
3. `--secure` enforces atomic single-fetch validation and stops malicious payloads at the boundary.
