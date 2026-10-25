# Module 26: Linux Kernel eBPF Verifier & Abstract Interpretation

Welcome to Module 26 of **0xLabs**. This module explores the architecture of the **Linux Kernel eBPF Verifier**—the single highest-impact vulnerability attack surface in the modern Linux kernel over recent years (responsible for CVE-2020-8835, CVE-2021-3490, CVE-2021-31440, CVE-2022-23222, and more).

---

## 1. Why eBPF is a Critical Kernel Attack Surface

eBPF (Extended Berkeley Packet Filter) allows unprivileged or low-privileged users to inject sandboxed programs into the Linux kernel to run at bare-metal speeds upon network events, tracepoints, or system calls.

Because these programs execute inside **Ring 0**, they must never crash the kernel or read/write arbitrary kernel memory.

To guarantee safety, the Linux kernel passes every bytecode program through an in-kernel static analyzer: **The eBPF Verifier**.

---

## 2. Abstract Interpretation & Bounds Tracking

The verifier does not run the program with concrete values; it performs **abstract interpretation**:
- Every register $R_0 \dots R_{10}$ is assigned an abstract state:
  - `type`: `SCALAR_VALUE`, `PTR_TO_MAP_VALUE`, `PTR_TO_STACK`, etc.
  - Range bounds: `umin_value`, `umax_value` (unsigned minimum/maximum values).
  - 32-bit sub-register bounds: `u32_min_value`, `u32_max_value`.
- When an instruction computes an address (e.g. `r1 = map_ptr + r2`), the verifier checks:
  $$\text{map\_offset} + \text{r2.umax\_value} < \text{map\_size}$$

---

## 3. The Root Cause: Bounds Desynchronization

When can an unprivileged user bypass the verifier?
When there is a **desynchronization** between:
1. What the **verifier believes** the register value is (e.g., $r_2 \in [0, 10]$).
2. What the **hardware CPU ALU actually computes** at runtime (e.g., $r_2 = 64$).

In CVE-2020-8835 and CVE-2021-3490, sub-register operations (ALU32, 32-bit moves, bitwise operations) updated the 32-bit bounds but erroneously propagated incorrect bounds to the 64-bit register representation.

As a result:
- The verifier validates the pointer calculation because it believes the offset is bounded by $10 < 64$.
- At runtime in Ring 0, the CPU uses the actual value ($64$), dereferencing past the end of the map buffer into adjacent sensitive kernel memory (e.g. leaking encryption keys, kernel pointers, or creds)!

---

## 4. Hands-On Verification

Run the test suite:
```bash
./test_ebpf.sh
```

Observe:
- In vulnerable mode: The simulated bounds desync causes the verifier to approve an out-of-bounds map read, leaking the adjacent `secret_kernel_token`.
- In secure mode: Correct interval bounds propagation detects the out-of-bounds pointer arithmetic (`offset 64 >= map size 64`) and halts verification with an explicit rejection.
