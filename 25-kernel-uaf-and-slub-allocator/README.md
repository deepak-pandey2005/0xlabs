# Module 25: Linux Kernel Heap Mechanics & The SLUB Allocator

Welcome to Module 25 of **0xLabs**. This module dives directly into the architecture of the Linux kernel memory allocator (**the SLUB allocator**) and demonstrates how **Kernel Use-After-Free (UAF)** vulnerabilities enable local privilege escalation (LPE).

---

## 1. How the Linux Kernel Allocates Heap Memory

Unlike user space which uses `glibc`'s `ptmalloc` (`malloc`/`free`), the Linux kernel manages dynamic memory using slab allocators (principally **SLUB** on modern Linux systems):

1. **Size Bins (`kmalloc-N`)**:
   General allocations via `kmalloc(size, GFP_KERNEL)` are rounded up to fixed power-of-two size classes (e.g., `kmalloc-32`, `kmalloc-64`, `kmalloc-128`, ..., `kmalloc-4096`).
2. **Dedicated Caches (`kmem_cache`)**:
   Performance-critical or high-frequency kernel objects have dedicated slab caches (e.g. `task_struct`, `cred`, `files_struct`, `inode_cache`).
3. **The Freelist**:
   Each slab contains multiple uniform slots. Free slots are chained in a singly-linked list. The pointer to the next free slot is stored *inside the unallocated slot itself* (`freelist pointer`).

---

## 2. Kernel Use-After-Free (UAF) Mechanics

When a kernel module or subsystem calls `kfree(ptr)`:
1. The memory slot is returned to the head of the corresponding `kmem_cache` freelist (LIFO order).
2. If the kernel driver retains a copy of `ptr` without setting it to `NULL`, `ptr` becomes a **dangling pointer**.
3. **Heap Spraying & Object Replacement**:
   If an unprivileged user process triggers another kernel operation that allocates a sensitive structure belonging to the *same size bin* (e.g., `kernel_credential_t` in `kmalloc-64`), the SLUB allocator hands out the exact same slot!
4. Stale writes through the dangling pointer now overwrite the fields of the sensitive object—such as modifying `cred->uid` from $1000$ to $0$ (`root`).

---

## 3. Kernel Mitigations: Freelist Hardening

To prevent attackers from corrupting the freelist pointer to achieve arbitrary kernel writes:
- **`CONFIG_SLAB_FREELIST_HARDENED`**:
  Instead of storing a raw 64-bit pointer, the kernel mangles the freelist pointer:
  $$\text{encoded\_ptr} = \text{ptr} \oplus \text{random\_slab\_cookie} \oplus \text{ptr\_addr}$$
  Because the random cookie is generated at boot and the pointer address is individualized for each slot, overwriting the pointer with a constant value causes `freelist_ptr_decode()` to produce an invalid address, immediately panicking the kernel before an arbitrary write can take place.
- **Reference Counting (`kref`)**:
  Modern kernel code uses `kref_get()` and `kref_put()` to ensure memory is only released when all active references across all threads reach zero.

---

## 4. Hands-On Verification

Run the test harness:
```bash
./test_slub.sh
```

Inspect:
1. How an unprivileged `user_session_t` and a root `kernel_credential_t` reuse the same 64-byte SLUB slot.
2. How modifying the stale session pointer alters `cred->uid`.
3. How `CONFIG_SLAB_FREELIST_HARDENED` scrambles freelist pointers into randomized 64-bit integers.
