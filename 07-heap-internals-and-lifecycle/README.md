# Module 07: Heap Internals & Lifecycle Management

While stack memory is managed automatically by function call frames, **Heap memory** is managed explicitly at runtime via dynamic allocators such as glibc's `ptmalloc` (`malloc`, `free`, `realloc`, `calloc`).

Understanding heap chunk metadata, recycling mechanics, and lifecycle bugs is critical for modern systems programming and vulnerability triage.

---

## 1. Anatomy of a 64-Bit Glibc Heap Chunk

In glibc, heap memory is allocated in discrete **chunks**. A chunk consists of an 8-byte (or 16-byte) metadata header immediately preceding the user data payload:

```
+-------------------------------------------------------------+
| prev_size (8 bytes)  - Size of preceding chunk if free      |
+-------------------------------------------------------------+
| size + flags (8 bytes)                                      |
|   - Bits 63..3 : Chunk size in bytes (multiples of 16)      |
|   - Bit 2 (A)  : NON_MAIN_ARENA (allocated from sub-arena)  |
|   - Bit 1 (M)  : IS_MMAPPED (allocated via direct mmap)    |
|   - Bit 0 (P)  : PREV_INUSE (1 = preceding chunk is busy)   |
+-------------------------------------------------------------+  <-- User Pointer returned by malloc()
| User Data Payload (8, 16, 32, ... bytes)                    |
| ...                                                         |
+-------------------------------------------------------------+
```

When a chunk is freed, the user payload region is repurposed to store forward (`fd`) and backward (`bk`) pointers that link the chunk into allocator free lists (tcache, fastbins, unsorted bin).

---

## 2. Common Heap Vulnerabilities Explained

### 2.1 Use-After-Free (UAF)
A Use-After-Free occurs when a pointer to heap memory is retained and dereferenced after `free()` has already returned the chunk to the allocator:
1. Object A is allocated and initialized.
2. Object A is freed, but the variable pointer is **not set to NULL** (a *dangling pointer*).
3. Object B is allocated. Because glibc caches freed chunks (tcache) for performance, Object B is assigned the **exact same memory address**.
4. The application reads or writes through the dangling pointer to Object A, inadvertently reading or modifying Object B's memory fields (Type Confusion).

### 2.2 Double-Free
Calling `free(ptr)` twice on the same memory block corrupts allocator bins. Modern glibc detects double-free via tcache count validation and per-thread canary cookies.

---

## 3. Hands-on Demonstrations

### Exercise 1: Run Standard Heap Inspection
```bash
make
./heap_demo
```
Observe:
1. How `inspect_chunk_header` reads the raw 8-byte header before the user pointer, showing allocation size and the `PREV_INUSE` bit.
2. How `admin_session` and `user_note` share the exact same memory address (`CHUNK RECYCLED`).
3. How writing `'A'` into `user_note->text` corrupted `admin_session->role` from `1` to `0x41414141`.

### Exercise 2: Catching Use-After-Free with AddressSanitizer (ASan)
Compile with AddressSanitizer and run:
```bash
./heap_demo_asan
```
Notice how ASan intercepts the UAF immediately:
- Reports `ERROR: AddressSanitizer: heap-use-after-free`.
- Shows exact file and line where the memory was originally allocated.
- Shows exact file and line where it was freed.
- Visualizes shadow memory (`fd` marks freed memory redzones).

---

## 4. Defensive Memory Management Patterns

1. **Defensive Pointer Zeroing:**
   Always assign `ptr = NULL` immediately following `free(ptr)`. Subsequent access will crash cleanly on a NULL-pointer dereference rather than silently corrupting state.
2. **Safe Deallocation Macro:**
   ```c
   #define SAFE_FREE(p) do { free(p); (p) = NULL; } while(0)
   ```
3. **Use Sanitizers in CI/CD:**
   Always run test suites built with `-fsanitize=address,undefined`.
4. **Modern Alternatives:**
   Adopt RAII patterns in C++ (`std::unique_ptr`, `std::shared_ptr`) or memory-safe languages (Rust) that enforce ownership semantics at compile time.
