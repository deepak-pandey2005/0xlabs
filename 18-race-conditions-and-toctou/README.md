# Module 18: Time-of-Check to Time-of-Use (TOCTOU) Race Conditions

In operating systems and systems software, **Time-of-Check to Time-of-Use (TOCTOU)** is a classic race condition vulnerability that occurs when a program checks a resource's state (such as file permissions or identity) and subsequently acts on that resource, but the resource changes between the two operations.

---

## 1. Anatomy of a File-Based TOCTOU

Consider the canonical path-based file writing pattern:

```c
// 1. TIME OF CHECK (TOC)
if (access(filepath, W_OK) == 0) {
    // 2. WINDOW OF VULNERABILITY
    // The scheduler may suspend this process; a concurrent process can alter the filesystem!
    
    // 3. TIME OF USE (TOU)
    FILE *f = fopen(filepath, "w");
    fputs(untrusted_data, f);
    fclose(f);
}
```

### The Symlink Swap Attack
1. The program validates `access(filepath, W_OK)`. At this instant, `filepath` points to a normal temporary file in `/tmp`.
2. Before `fopen()` runs, an unprivileged user deletes the temporary file and creates a **symbolic link** pointing to a critical system file (e.g., `/etc/shadow` or a database configuration).
3. When `fopen(filepath, "w")` runs, it resolves the symlink and **truncates/overwrites the target file**, effectively granting unauthorized modification privileges!

---

## 2. Modern Defensive Systems & Atomic File Operations

Operating systems provide atomic system calls specifically designed to eliminate TOCTOU windows:

| Insecure Pattern | Atomic Secure Replacement | Why It Eliminates the Race |
|---|---|---|
| `access() + fopen()` | `open(path, O_CREAT \| O_EXCL \| O_NOFOLLOW, mode)` | Atomic kernel check: `O_NOFOLLOW` fails if path is a symlink; `O_EXCL` fails if the file already exists. |
| Path-based `chmod(path)` | Descriptor-based `fchmod(fd)` | Operates on the already-opened file descriptor; cannot be swapped via symlink. |
| Path-based `stat(path)` | Descriptor-based `fstat(fd)` | Validates the inode of the exact open handle. |

### Kernel-Level Mitigations
Linux provides built-in sysctl protections in `/proc/sys/fs/`:
- `fs.protected_symlinks = 1`: Disallows following symlinks in world-writable sticky directories (`/tmp`) unless the follower owns the symlink or the target.
- `fs.protected_hardlinks = 1`: Restricts hardlink creation to files owned by the user.

---

## 3. Hands-on Demonstrations

### Exercise 1: Run the Automated TOCTOU Race Simulation
```bash
make
./race_test.sh
```

Follow the three phases:
1. **Target Setup:** Creates a protected mock configuration file (`protected_file.txt`).
2. **Race Exploitation:** Executes the vulnerable binary with a simulated preemption window. During the window, the script creates a symlink `target_link -> protected_file.txt`. The vulnerable program follows the symlink and corrupts the protected configuration.
3. **Atomic Defense:** Runs the secure implementation with `O_NOFOLLOW | O_EXCL`. The atomic flags detect the existing symlink and abort (`EEXIST`), keeping the protected configuration completely intact.
