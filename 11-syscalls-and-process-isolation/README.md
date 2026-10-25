# Module 11: Linux System Calls & Process Isolation (SECCOMP)

The Linux system call interface represents the ultimate boundary between unprivileged User Space (Ring 3) and the Kernel (Ring 0).

Understanding how system calls execute at the assembly level and how to isolate processes using kernel sandboxing mechanisms like **seccomp** is fundamental to modern containerization and exploit mitigation.

---

## 1. The x86_64 Syscall Interface

In 64-bit x86 Linux, user space transitions into kernel mode via the CPU `syscall` instruction.

### Register Convention
- **Syscall Number:** Stored in `%rax` (e.g. `1` = `sys_write`, `39` = `sys_getpid`, `60` = `sys_exit`).
- **Arguments (1 through 6):** `%rdi`, `%rsi`, `%rdx`, `%r10`, `%r8`, `%r9`.
  *(Note: Unlike the standard C calling convention which uses `%rcx` for the 4th argument, syscalls use `%r10` because the CPU hardware automatically clobbers `%rcx` to store the user return `%rip`).*
- **Return Value:** Placed in `%rax`. Negative values in `[-4095, -1]` represent errno error codes.

---

## 2. SECCOMP (Secure Computing Mode)

**SECCOMP** is a Linux kernel security feature designed to drastically reduce attack surface by restricting which system calls an application is allowed to make.

### SECCOMP_MODE_STRICT
Once strict seccomp mode is enabled via `prctl(PR_SET_SECCOMP, SECCOMP_MODE_STRICT)`:
- Only **4 system calls** are permitted:
  1. `read()`
  2. `write()`
  3. `_exit()` (and `sigreturn`)
  4. `sigreturn()`
- If the thread invokes **any other system call** (such as `open()`, `fork()`, `execve()`, or `socket()`), the Linux kernel immediately terminates the process with `SIGKILL`.

### SECCOMP_MODE_FILTER (seccomp-bpf)
Used in Docker containers, systemd sandboxing, and Chromium:
- Uses Berkeley Packet Filter (BPF) programs loaded into the kernel.
- Inspects system call numbers and argument pointers/values at runtime.
- Allows fine-grained policies (e.g., allow `openat` for reading only, deny network sockets).

---

## 3. Hands-on Demonstrations

### Exercise 1: Run the Syscall & Seccomp Suite
```bash
make
./run_syscall_demo.sh
```
Follow the output through three demonstrations:
1. **Raw Assembly Syscalls:** `sys_write` and `sys_getpid` executed via inline assembly, bypassing glibc wrappers entirely.
2. **Clean Sandboxed Execution:** Permitted operations (`sys_write` and `sys_exit`) complete within the strict sandbox.
3. **Sandbox Enforcement:** Attempting to call `sys_getpid` inside the sandbox triggers immediate kernel termination (`SIGKILL`, status `137`).

---

## 4. Modern Defensive Systems

1. **Principle of Least Privilege:** Drop capabilities (`cap_set_proc`) and restrict syscalls before processing untrusted input.
2. **Container Security:** Docker default seccomp profile blocks ~44 dangerous syscalls (such as `reboot`, `sys_ptrace`, `kexec_load`).
3. **Chromium Renderer Sandbox:** Renders untrusted web pages inside a strict seccomp jail, ensuring that even if an attacker achieves arbitrary code execution, they cannot execute `execve` or access the host filesystem.
