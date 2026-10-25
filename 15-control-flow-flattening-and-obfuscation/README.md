# Module 15: Control Flow Flattening (CFF) & Reverse Engineering De-obfuscation

In commercial software protection, DRM solutions, and malware binaries, authors frequently employ **Code Obfuscation** to impede static reverse engineering.

Among these methods, **Control Flow Flattening (CFF)** (originally formalized by Chenxi Wang and implemented in tools like OLLVM and Tigress) is one of the most effective techniques used to confuse disassemblers and decompilers (IDA Pro, Ghidra).

---

## 1. How Control Flow Flattening Works

### Normal Linear Control Flow
In standard code, control flow has clear hierarchical branching:
```
[ Basic Block A: Init ]
           |
           v
[ Basic Block B: Loop Condition ] <-----+
      /                  \              |
  (False)              (True)           |
    /                      \            |
   v                        v           |
[ Block D: Exit ]      [ Block C: Body ]-+
```

### Flattened Control Flow (State Machine)
Flattening removes all direct edge relationships between basic blocks:
1. All basic blocks are placed inside a single loop containing a massive `switch(state)` dispatcher.
2. Every basic block finishes by setting the `state` variable to the next target block and jumping back to the central dispatcher.
3. In a decompiler, the control flow graph (CFG) is flattened into a 2D hub-and-spoke star topology, destroying loop hierarchies and producing a maze of switch cases.

```
                  +--------------------------+
                  | Dispatcher: switch(state)| <----+
                  +--------------------------+      |
                   /      |         |      \        |
                  v       v         v       v       |
               [BB 0x10] [BB 0x20] [BB 0x30] [BB 0x40] --+
```

---

## 2. Reverse Engineering & De-flattening Methodologies

To analyze flattened binaries, reverse engineers use three primary strategies:

| Strategy | Methodology | Key Tools |
|---|---|---|
| **1. Dynamic State Tracing** | Hook the dispatcher switch instruction and record the sequence of `state` transitions at runtime to recover the execution path. | GDB Python, Frida, Qiling |
| **2. Symbolic Execution** | Symbolically execute each basic block from dispatcher exit to dispatcher re-entry to calculate the next state algebraically without running the full program. | angr, Triton, Miasm |
| **3. Intermediate Representation (IR) Optimization** | Reconstruct the CFG by replacing dispatcher jumps with direct branch edges using compiler optimization passes. | D-Flat, Ghidra P-Code scripts |

---

## 3. Hands-on Demonstrations

### Exercise 1: Run the Comparison & CFG Reconstruction
```bash
make
./trace_cfg.sh
```

Follow the two phases:
1. **Disassembly Comparison:**
   - Look at `compute_hash_original`: notice standard sequential instructions (`strlen`, `add`, `jl`).
   - Look at `compute_hash_flattened`: notice the cascade of `cmp [rbp-0x4], 0x...` comparisons and jump tables that break traditional CFG views.
2. **Dynamic CFG Reconstruction:**
   - The script uses GDB's Python API to intercept state transitions inside the dispatcher loop, reconstructing the exact semantic flow (`INIT -> LOOP_CHECK -> LOOP_BODY -> BRANCH -> EXIT`).
