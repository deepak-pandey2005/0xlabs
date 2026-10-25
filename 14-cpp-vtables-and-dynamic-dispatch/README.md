# Module 14: C++ Virtual Method Tables (vtables) & Dynamic Dispatch

In modern systems, browsers (V8/Blink, SpiderMonkey), operating systems, and game engines are written extensively in C++. Understanding how compilers lay out C++ objects in memory, how polymorphism works at runtime, and how indirect calls execute in assembly is an essential reverse engineering skill.

---

## 1. The Itanium C++ ABI Memory Model

When a C++ class defines at least one `virtual` method, the compiler attaches a hidden pointer to the beginning of the object: the **`_vptr`** (Virtual Table Pointer).

```
Object in Memory (Stack / Heap)          vtable in .rodata (Read-Only)
+-------------------------------+        +-------------------------------+
| _vptr (8 bytes at offset 0)  | -----> | vtable[0] : Destructor        |
+-------------------------------+        +-------------------------------+
| service_id (4 bytes)          |        | vtable[1] : &execute_task()   |
+-------------------------------+        +-------------------------------+
| [padding / derived fields]    |        | vtable[2] : &authenticate()   |
+-------------------------------+        +-------------------------------+
```

When an object is instantiated:
1. The constructor sets `this->_vptr` to the class's vtable in `.rodata` (`vtable for BaseService` or `vtable for AdminService`).
2. Even if a function only holds a pointer to `BaseService* obj`, invoking `obj->execute_task()` reads `obj->_vptr` and jumps to the derived implementation!

---

## 2. Dynamic Dispatch in Assembly

When reversing C++ binaries in GDB or Ghidra, virtual method calls do not produce direct `call <symbol>` instructions. Instead, they produce an indirect call pattern:

```assembly
mov    rax, QWORD PTR [rbp-0x20]   ; Load 'this' pointer (obj address)
mov    rax, QWORD PTR [rax]        ; Dereference offset 0 to get _vptr
add    rax, 0x10                   ; Index into vtable slot (+0x10 for 2nd method)
mov    rdx, QWORD PTR [rax]        ; Load the target function pointer into %rdx
mov    rdi, QWORD PTR [rbp-0x20]   ; Pass 'this' as 1st argument in %rdi
call   rdx                         ; Indirect call through register!
```

---

## 3. Security Implications & Modern Defenses

### vtable Hijacking & Type Confusion
Because `_vptr` is a pointer stored inside the object's memory:
- If a memory safety bug (such as Use-After-Free or heap buffer overwrite) corrupts the object's `_vptr`, calling any virtual function executes an indirect jump through the corrupted pointer.

### Forward-Edge Control Flow Integrity (CFI)
To defend against indirect call manipulation, modern compilers introduce **Forward-Edge CFI**:
- **Clang CFI (`-fsanitize=cfi`)**: At compile time, the compiler groups all valid vtables for a class hierarchy into a contiguous memory table. Before any `call rdx`, it inserts a bitmask check verifying that `rdx` belongs to a legitimate, authorized vtable.
- If an attacker tampers with the `_vptr`, the CFI check fails and aborts before `call rdx` is executed.

---

## 4. Hands-on Demonstrations

### Exercise 1: Run the vtable Inspection Suite
```bash
make
./run_vtable_demo.sh
```
Observe:
1. `nm -C` resolving demangled symbols: `vtable for BaseService` and `vtable for AdminService`.
2. Inspecting the raw 64-bit hexadecimal words inside the object at runtime.
3. The GDB disassembly showing the exact `mov rax, [rax] ; add rax, 0x10 ; call rdx` dispatch sequence.
