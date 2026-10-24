# Module 06: Format String Vulnerabilities & Memory Disclosure

The format string bug is one of the classic vulnerabilities in C/C++ programming. It occurs when untrusted user input is passed directly as the format argument to a formatted output function:

```c
// VULNERABLE:
printf(user_input);

// SECURE:
printf("%s", user_input);
```

---

## 1. How Variadic Functions Work Under the Hood

Functions with variable argument lists (such as `printf`, `sprintf`, `fprintf`) rely on the `<stdarg.h>` API (`va_start`, `va_arg`, `va_end`).

In x86_64 System V AMD64 ABI:
- **1st argument (`fmt`):** Stored in register `%rdi`.
- **Arguments 2 to 6:** Stored in `%rsi`, `%rdx`, `%rcx`, `%r8`, `%r9`.
- **Arguments 7+:** Pushed sequentially onto the stack.

When `printf` parses a conversion specifier (`%x`, `%d`, `%p`, `%s`):
1. It queries `va_arg` for the next argument.
2. If the programmer provided fewer arguments than format specifiers (e.g. `printf(user_input)` where 0 additional arguments were provided), the CPU reads whatever values happen to currently reside in the remaining registers and then **walks up the caller's stack frame**!

---

## 2. Common Conversion Specifiers & Their Impact

| Specifier | What It Interprets | Security Impact |
|---|---|---|
| `%p` / `%lx` | Pointer / 64-bit hex word | **Memory Disclosure:** Leaks stack addresses, stack canary values, ASLR base addresses, or confidential local variables. |
| `%s` | Null-terminated string pointer | **Arbitrary Read:** Dereferences the address stored in the stack slot. If invalid, causes a SIGSEGV crash. |
| `%n` | Pointer to integer | **Arbitrary Write:** Writes the count of characters printed so far to the address stored in the argument. |
| `%<N>$<spec>` | Direct Parameter Access | **Targeted Stack Read:** Jumps directly to the $N$-th argument on the stack (e.g. `%23$lx`). |

---

## 3. Hands-on Demonstrations

### Exercise 1: Run the Leak Demonstration
```bash
make
./leak_demo.sh
```

Notice the four progressive demonstrations:
1. **Benign Input:** Strings without `%` display normally.
2. **Register/Stack Walking:** Passing `%p %p %p %p %p %p` walks `%rsi`, `%rdx`, `%rcx`, `%r8`, `%r9`, and stack slot 1.
3. **Targeted Secret Leak:** Using positional specifier `%23$lx` jumps directly to the 64-bit secret token on the stack (`0xDEADBEEFCAFE1337`), bypassing all surrounding bytes.
4. **Defensive Fix:** Running with `--safe` passes the input as data to `printf("%s", ...)`. All specifiers are treated as literal characters.

---

## 4. Compiler Mitigations & Best Practices

Modern toolchains offer proactive flags to eradicate format string bugs:

1. **`-Wformat=2 -Werror=format-security`**:
   Instructs GCC/Clang to treat any non-literal format string as a fatal build error.
2. **FORTIFY_SOURCE (`-D_FORTIFY_SOURCE=2`)**:
   Enforces checks preventing `%n` writes to read-only or untrusted memory.
3. **Rule of Thumb:** Never omit the format string literal. Always use:
   ```c
   printf("%s", str);
   syslog(LOG_INFO, "%s", message);
   ```
