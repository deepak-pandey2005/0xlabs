#!/usr/bin/env bash
set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$DIR"

echo "=========================================================="
echo " 0xLabs - Module 27: Symbolic Execution & Constraint Solving"
echo "=========================================================="

echo "[*] Step 1: Compiling target challenge binary..."
make clean
make all

echo ""
echo "[*] Step 2: Testing manual guess (Expected Failure)..."
echo "----------------------------------------------------------"
./auth_challenge "ABCD" || true

echo ""
echo "[*] Step 3: Executing Symbolic Solver (Resolving Path Constraints)..."
echo "----------------------------------------------------------"
python3 symbolic_solver.py

echo ""
echo "=========================================================="
echo " [✓] Symbolic Execution Demo Completed Successfully!"
echo "=========================================================="
