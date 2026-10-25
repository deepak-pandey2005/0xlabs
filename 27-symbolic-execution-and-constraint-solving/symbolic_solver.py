#!/usr/bin/env python3
"""
0xLabs - Module 27: Symbolic Execution & Constraint Solver

Simulates symbolic execution and constraint solving over the disassociated
path conditions extracted from auth_challenge.

Constraints extracted from binary disassembly:
  Path Condition 1: k0 ^ 0x1E == 0x55
  Path Condition 2: k1 * 5 + 17 == 362
  Path Condition 3: k2 ^ k0 == 0x12
  Path Condition 4: k3 * 4 + k1 == 201
"""

import sys
import subprocess
import time

def solve_symbolic_constraints():
    print("[*] Symbolic Execution Engine Initialized")
    print("[*] Symbolic variables defined: k0, k1, k2, k3 (each 8-bit AST)")
    print("[*] Formulating accumulated path constraints:")
    print("    C1: (k0 ^ 0x1E) == 0x55")
    print("    C2: (k1 * 5 + 17) == 362")
    print("    C3: (k2 ^ k0) == 0x12")
    print("    C4: (k3 * 4 + k1) == 201")

    start_time = time.time()

    # Step 1: Solve C1 for k0
    # k0 ^ 0x1E == 0x55 => k0 = 0x55 ^ 0x1E
    k0 = 0x55 ^ 0x1E
    print(f"\n[+] Constraint C1 solved: k0 = {k0} (ASCII: '{chr(k0)}')")

    # Step 2: Solve C2 for k1
    # k1 * 5 + 17 == 362 => k1 = (362 - 17) // 5
    k1 = (362 - 17) // 5
    print(f"[+] Constraint C2 solved: k1 = {k1} (ASCII: '{chr(k1)}')")

    # Step 3: Solve C3 for k2 using solved k0
    # k2 ^ k0 == 0x12 => k2 = 0x12 ^ k0
    k2 = 0x12 ^ k0
    print(f"[+] Constraint C3 solved: k2 = {k2} (ASCII: '{chr(k2)}')")

    # Step 4: Solve C4 for k3 using solved k1
    # k3 * 4 + k1 == 201 => k3 = (201 - k1) // 4
    k3 = (201 - k1) // 4
    print(f"[+] Constraint C4 solved: k3 = {k3} (ASCII: '{chr(k3)}')")

    elapsed_ms = (time.time() - start_time) * 1000
    solved_key = chr(k0) + chr(k1) + chr(k2) + chr(k3)

    print(f"\n[✓] Satisfying model found in {elapsed_ms:.2f} ms!")
    print(f"[✓] Recovered Key: '{solved_key}' (Bytes: {[hex(c) for c in [k0, k1, k2, k3]]})")

    return solved_key

def verify_against_target(key):
    print(f"\n[*] Executing target binary './auth_challenge {key}'...")
    proc = subprocess.run(["./auth_challenge", key], capture_output=True, text=True)
    print("------------------------------------------------------------")
    print(proc.stdout.strip())
    print("------------------------------------------------------------")

    if proc.returncode == 0:
        print("[✓] TARGET CRACKED: Symbolic execution successfully produced valid authentication!")
    else:
        print("[!] Validation failed against target binary.")
        sys.exit(1)

if __name__ == "__main__":
    key = solve_symbolic_constraints()
    verify_against_target(key)
