# Module 01: ELF Binary Anatomy

Understanding how executable binaries are structured at the byte level is the foundational bedrock of reverse engineering, binary analysis, and systems security.

On Linux systems, compiled binaries use the **ELF** format: **Executable and Linkable Format**.

---

## 1. The Anatomy of an ELF Binary

An ELF binary is divided into two distinct views:
1. **Linking View (Sections):** Used by linkers (`ld`), debuggers (`gdb`), and disassemblers to understand logical blocks of code and data.
2. **Execution View (Segments):** Used by the kernel's program loader (`execve`) to map contiguous regions of memory with specific hardware permissions (Read, Write, Execute).

```
+------------------------------------+  Low Addresses (0x400000 approx)
| ELF Header                         |  Identifies architecture, entrypoint
+------------------------------------+
| Program Headers (Segments)         |  Tells kernel how to map memory
+------------------------------------+
| .text section (Code)               |  R-X: CPU instructions
+------------------------------------+
| .rodata section (Constants)        |  R--: Strings and consts
+------------------------------------+
| .data section (Initialized Globals)|  RW-: Initialized variables
+------------------------------------+
| .bss section (Zeroed Globals)      |  RW-: Zero-initialized variables
+------------------------------------+
| Section Headers & Symbol Table     |  Metadata, function & variable names
+------------------------------------+
```

---

## 2. Key ELF Sections Explained

| Section | Permissions | Content | Present on Disk? |
|---|---|---|---|
| `.text` | `R-X` (Read, Execute) | Compiled machine instructions for functions (`main`, `print_memory_layout`). | Yes |
| `.rodata` | `R--` (Read Only) | Read-only data such as string literals (`"Hello world"`). Writing triggers SIGSEGV. | Yes |
| `.data` | `RW-` (Read, Write) | Global/static variables with non-zero initial values (`int counter = 42;`). | Yes |
| `.bss` | `RW-` (Read, Write) | Uninitialized or zero-initialized globals (`int arr[1024];`). | No (allocated by loader) |
| `.symtab` | None | Symbol table containing function names, variable locations, and types. | Yes (unless stripped) |
| `.strtab` | None | String table holding symbol names referenced by `.symtab`. | Yes (unless stripped) |

---

## 3. Hands-On Exercises

### Exercise 1: Run the Automated Inspection Script
```bash
make
./inspect.sh
```

### Exercise 2: Inspecting the ELF Header
Run:
```bash
readelf -h anatomy
```
- **Magic:** `7f 45 4c 46` (`\x7fELF`) - The magic bytes that identify the file format.
- **Class:** `ELF64` (64-bit architecture).
- **Entry point address:** The virtual address where the CPU begins execution (typically `_start`, which initializes libc and calls `main`).

### Exercise 3: Section Headers and Addresses
Run:
```bash
readelf -S anatomy
```
Notice the `Addr` (runtime virtual memory address) and `Off` (offset on disk) for each section.

### Exercise 4: Stripped vs Unstripped Binaries
Compare the symbol tables:
```bash
# Unstripped binary contains function and variable names
nm anatomy

# Stripped binary removes .symtab to reduce size and hinder static analysis
nm anatomy_stripped
```
In reverse engineering, stripped binaries are the norm in production software; reverse engineers must identify functions by their assembly patterns rather than symbol names.
