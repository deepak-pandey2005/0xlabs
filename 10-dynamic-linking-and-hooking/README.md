# Module 10: Dynamic Linking, PLT/GOT & API Hooking

When a program calls functions from external libraries (such as `printf`, `strcmp`, or `malloc` in `libc.so`), the physical addresses of those functions are not known at compile time.

Linux ELF binaries use the **Procedure Linkage Table (PLT)** and **Global Offset Table (GOT)** to resolve and bind external symbols dynamically at runtime.

---

## 1. How PLT and GOT Work

```
User Code: call strcmp@plt
         |
         v
+------------------------------------+
| strcmp@plt (Stub in .plt)          |  jmp [strcmp@got]
+------------------------------------+
         |
         v
+------------------------------------+
| strcmp@got (Entry in .got.plt)     |
|   - 1st call: Points back to PLT   |  Triggers _dl_runtime_resolve() in ld.so
|   - ld.so finds real libc address  |  Overwrites strcmp@got with real address!
|   - 2nd call: Direct jump to libc! |  Zero overhead for subsequent calls!
+------------------------------------+
```

### Relocations Table
Dynamic relocations can be audited with:
```bash
readelf -r app
```
Look for `R_X86_64_JUMP_SLOT` entries corresponding to functions in the `.got.plt` section.

---

## 2. Function Interposition & Hooking with `LD_PRELOAD`

On Linux, the dynamic linker (`ld.so`) supports an environment variable named `LD_PRELOAD`.

If set, the dynamic linker loads the specified shared libraries **before** any other library (including standard `libc.so.6`). If your library defines a function with the same symbol name (e.g., `strcmp`), the dynamic linker binds all calls to **your version** instead!

### Using `dlsym(RTLD_NEXT, ...)`
To inspect arguments without breaking original functionality:
```c
#define _GNU_SOURCE
#include <dlfcn.h>

int strcmp(const char *s1, const char *s2) {
    // Locate the REAL strcmp in subsequent libraries (libc)
    int (*real_strcmp)(const char *, const char *) = dlsym(RTLD_NEXT, "strcmp");

    // Intercept and examine values
    printf("Intercepted comparison: %s vs %s\n", s1, s2);

    // Call the original function
    return real_strcmp(s1, s2);
}
```

---

## 3. Hands-on Demonstrations

### Exercise 1: Run the Complete Demonstration
```bash
make
./run_hook_demo.sh
```
Observe:
1. `readelf -r` displaying the dynamic relocation entries for `strcmp` and `puts`.
2. Running `./app "test1234"` fails normally.
3. Running with `LD_PRELOAD=./hook_strcmp.so ./app "test1234"` intercepts the function call and exposes the secret target key:
   `SUPER_SECRET_0xKEY_2026`.
4. Authenticating with the discovered key yields access granted.

---

## 4. Security & Engineering Applications

1. **Debugging & Profiling:** Tracking memory allocations and detecting leaks (`malloc` / `free` interposition).
2. **Dynamic Instrumentation:** Sandboxing, monitoring suspicious behavior, and security telemetry.
3. **Defense Against Hooking:**
   - Static linking (`-static` eliminates PLT/GOT and `LD_PRELOAD`).
   - SUID binaries ignore `LD_PRELOAD` automatically for non-root users.
   - Calling direct system calls (`syscall()`) rather than glibc wrappers.
