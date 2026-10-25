# Module 27: Symbolic Execution & Constraint Solving

Welcome to Module 27 of **0xLabs**. This module introduces **Symbolic Execution and SMT Constraint Solving**—essential techniques in modern automated reverse engineering, vulnerability discovery, and exploit synthesis.

---

## 1. Concrete vs Symbolic Execution

In standard (concrete) execution:
- Program inputs are specific byte values (e.g. `"ABCD"`).
- The CPU computes concrete arithmetic results and follows a single execution path.

In **Symbolic Execution**:
- Inputs are replaced with **symbolic variables** ($\alpha_0, \alpha_1, \dots$).
- As the program executes, intermediate calculations produce symbolic expressions (Abstract Syntax Trees).
- At every conditional jump (`je`, `jne`, `jg`), the execution engine branches into multiple paths and records the necessary conditions (**Path Constraints**).
- To reach a specific target state (e.g., bypassing an authentication check or reaching a crash site), the accumulated path formula is passed to an **SMT (Satisfiability Modulo Theories) solver** (such as Z3). The solver either proves unreachability or outputs concrete input bytes satisfying all equations.

---

## 2. Challenge Overview

In this lab, [auth_challenge.c](file:///home/deepak/OPENSOURCE/0xlabs/27-symbolic-execution-and-constraint-solving/auth_challenge.c) enforces a 4-byte key check with interlinked non-linear equations:
1. $k_0 \oplus 0x1E == 0x55$
2. $k_1 \times 5 + 17 == 362$
3. $k_2 \oplus k_0 == 0x12$
4. $k_3 \times 4 + k_1 == 201$

To brute-force all possible 4-byte combinations requires $2^{32} \approx 4.29 \times 10^9$ evaluations.
In contrast, [symbolic_solver.py](file:///home/deepak/OPENSOURCE/0xlabs/27-symbolic-execution-and-constraint-solving/symbolic_solver.py) traverses the constraints directly and produces the solution in under $1$ millisecond ($0.04$ ms).

---

## 3. Hands-On Verification

Run the demonstration script:
```bash
./run_symbolic_demo.sh
```

Observe:
1. An unguided guess (`"ABCD"`) immediately fails.
2. The symbolic solver resolves the AST constraints and derives the key: `'KEY!'`.
3. The recovered key is supplied to the target binary, unlocking `0xLABS-SYMBOLIC-SOLVED`.
