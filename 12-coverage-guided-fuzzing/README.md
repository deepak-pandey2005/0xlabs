# Module 12: Coverage-Guided Fuzzing & Crash Triage with LibFuzzer

In modern software security and vulnerability research, **Coverage-Guided Fuzzing** is the industry standard methodology for automatically discovering memory safety vulnerabilities, parsing bugs, and logic flaws.

Instead of testing random inputs blindly (black-box fuzzing), coverage-guided fuzzers instrument binary basic blocks during compilation. When a generated input discovers new code paths, the fuzzer preserves it in a dynamic **corpus** and mutates it further.

---

## 1. How LibFuzzer & AddressSanitizer Work Together

```
                  +-----------------------------------+
                  |           Seed Corpus             |
                  +-----------------------------------+
                                    |
                                    v
+-------------------> [ Mutation Engine (LibFuzzer) ]
|                                   |
|                                   v
|                     [ Target Function (Harness) ]
|                                   |
|   New Branch Coverage?            v
+-- YES (Save to Corpus)  [ Sanitizer Shadow Memory ]
                                    |
                            Memory Violation?
                                    |
                             YES ---+---> [ CRASH ARTIFACT CREATED ]
                                          - Writes: ./crash-<hash>
                                          - Reports file, line & stack trace
```

### Key Components
1. **`-fsanitize=fuzzer`**: Instruments branch edges with 8-bit counters and links the LLVM in-process fuzzing engine.
2. **`-fsanitize=address` (ASan)**: Surrounds all heap allocations and stack frames with poisoned **redzones**. Any out-of-bounds read or write hits a redzone and immediately aborts with an actionable crash report.
3. **The Fuzz Target Harness**: Implements `LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size)`.

---

## 2. Anatomy of the Target Application

In this lab, [`packet_parser.c`](./packet_parser.c) simulates a real-world binary network parser:
- Verifies a 4-byte magic signature (`"PKT1"`).
- Decodes packet type (`TYPE_PING`, `TYPE_DATA`, `TYPE_TEXT`).
- For `TYPE_TEXT`, allocates a 32-byte destination buffer:
  ```c
  char *text_buffer = (char *)malloc(32);
  ```
- **The Vulnerability:** The developer enforced a flawed validation check (`payload_len > 256`) instead of `sizeof(text_buffer)`. When a packet with 33 to 256 bytes arrives, `memcpy` writes past the end of the 32-byte heap buffer!

---

## 3. Hands-on Fuzzing & Crash Triage Workflow

### Exercise 1: Run the Complete Automated Workflow
```bash
make
./run_fuzzing.sh
```

Follow the four phases executed by the script:
1. **Corpus Initialization:** Generates a minimal valid seed (`PKT1\x01\x04TEST`) to bootstrap the fuzzer.
2. **Fuzzing Execution:** LibFuzzer executes hundreds of thousands of mutations per second. Within seconds, it mutates the payload length past 32 bytes and crashes with `heap-buffer-overflow`.
3. **Crash Artifact Creation:** LibFuzzer dumps the exact input that triggered the bug to disk:
   `./crash-<sha1_hash>`.
4. **Standalone Reproduction:** The script executes [`standalone_reproducer`](./standalone_reproducer.c) against the crash artifact, producing the exact ASan diagnostic:
   ```
   ==8057==ERROR: AddressSanitizer: heap-buffer-overflow on address 0x7b6d3e9e0060
   WRITE of size 33 at 0x7b6d3e9e0060 thread T0
       #1 in parse_packet packet_parser.c:52:9
   0x7b6d3e9e0060 is located 0 bytes after 32-byte region
   ```

### Exercise 2: Debugging the Crash in GDB
To inspect the crash interactively under GDB:
```bash
gdb --args ./standalone_reproducer crash-*
(gdb) run
(gdb) backtrace
```

---

## 4. Best Practices for Writing Fuzz Targets

1. **Keep it In-Memory:** Avoid file I/O, network sockets, or `system()` inside `LLVMFuzzerTestOneInput`.
2. **Deterministic & Stateless:** Reset global states between runs so identical inputs always produce identical code paths.
3. **Provide High-Quality Seed Corpora:** A valid seed packet dramatically speeds up exploration by bypassing initial magic-byte checks immediately.
4. **Differential Fuzzing:** Compare the output of two different implementations (e.g. OpenSSL vs LibreSSL) with the same input to find subtle logic discrepancies.
