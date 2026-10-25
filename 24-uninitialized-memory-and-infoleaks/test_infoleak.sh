#!/usr/bin/env bash
set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$DIR"

echo "=========================================================="
echo " 0xLabs - Module 24: Struct Padding & Uninitialized Leaks"
echo "=========================================================="

echo "[*] Step 1: Compiling binaries..."
make clean
make all

echo ""
echo "[*] Step 2: Running Standard Binary (Demonstrating Padding Leak)..."
echo "----------------------------------------------------------"
./infoleak_lab

echo ""
echo "[*] Step 3: Running with Compiler Auto-Zeroing (-ftrivial-auto-var-init=zero)..."
echo "----------------------------------------------------------"
./infoleak_auto_zero --vuln

echo ""
echo "=========================================================="
echo " [✓] Information Leak Analysis Completed Successfully!"
echo "=========================================================="
