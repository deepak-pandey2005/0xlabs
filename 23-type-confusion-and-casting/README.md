# Module 23: C++ Type Confusion & Unsafe Downcasting

Welcome to Module 23 of **0xLabs**. This module focuses on C++ object-oriented vulnerabilities, specifically **Type Confusion**—one of the most critical classes of zero-day vulnerabilities in modern web browsers (V8, WebKit, SpiderMonkey), JVM/CLR engines, and enterprise C++ software.

---

## 1. What is Type Confusion?

Type Confusion occurs when a program allocates or initializes an object of type $A$, but subsequent code accesses, dereferences, or calls methods on that memory location as if it were of an incompatible type $B$.

In high-level languages with weak type systems or unsafe downcasting:
- **Field Reinterpretation**: Integer counts or strings in type $A$ are reinterpreted as raw pointers, capability masks, or function pointers in type $B$.
- **Vtable / Dynamic Dispatch Corruption**: If type $B$ expects a vtable with different method indices than type $A$, an indirect virtual call (`call [rax + 0x10]`) jumps to an unintended function.
- **Out-of-Bounds Memory Access**: If `sizeof(B) > sizeof(A)`, accessing fields of $B$ reads and writes into adjacent heap chunks or stack frames!

---

## 2. Unsafe Downcasting: `static_cast` vs `dynamic_cast`

C++ provides multiple cast operators with drastically different safety guarantees:

| Operator | Mechanism | Runtime Overhead | Safety |
|---|---|---|---|
| `static_cast<T*>` | Compile-time pointer arithmetic | Zero cost ($0$ instructions) | **Unsafe**: Assumes programmer is 100% correct. No runtime check. |
| `dynamic_cast<T*>` | Inspects Runtime Type Information (RTTI) in the vtable | Small cost (vtable lookup) | **Safe**: Returns `nullptr` (or throws `std::bad_cast`) if types do not match. |

When performance-sensitive codebases (such as browser DOM nodes or JIT compilers) replace `dynamic_cast` with `static_cast` for speed without airtight invariant validation, type confusion vulnerabilities emerge.

---

## 3. Hands-On Demonstration

In this module, `TextDocument` (48 bytes) and `AdminSession` (64 bytes) both inherit from `BaseEntity`.

1. **Vulnerable Flow (`--vuln`)**:
   Passing a `TextDocument` to `process_admin_action_vulnerable()`:
   - Uses `static_cast<AdminSession*>`.
   - The document word count (`char_count`) is reinterpreted as the 64-bit `privilege_mask`, accidentally unlocking admin privileges!
   - Accessing `session->audit_callback` reads 8 bytes beyond the end of `TextDocument`, loading adjacent stack values as a function pointer and causing control-flow corruption.

2. **Secure Flow (`--secure`)**:
   - Enforces an explicit object type discriminator tag.
   - Uses `dynamic_cast<AdminSession*>` to query the Itanium C++ RTTI metadata.
   - Malicious or confused casts return `nullptr` and are rejected before execution.

3. **Compiler Sanitizer Detection**:
   - Clang's `-fsanitize=undefined` (specifically `-fsanitize=vptr`) catches illegal downcasts and memory layout mismatches at runtime, terminating execution with an informative diagnostic.

---

## 4. Execution

To run the complete automated test suite:
```bash
./run_type_confusion.sh
```
