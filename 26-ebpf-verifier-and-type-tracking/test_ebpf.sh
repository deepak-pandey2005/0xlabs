#!/usr/bin/env bash
set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$DIR"

echo "=========================================================="
echo " 0xLabs - Module 26: Linux Kernel eBPF Verifier Logic"
echo "=========================================================="

echo "[*] Step 1: Compiling ebpf_verifier_mock..."
make clean
make all

echo ""
echo "[*] Step 2: Testing Vulnerable Verifier Bounds Desync..."
echo "----------------------------------------------------------"
./ebpf_verifier_mock

echo ""
echo "[*] Step 3: Testing Secure Verifier with Boundary Enforcement..."
echo "----------------------------------------------------------"
./ebpf_verifier_mock --secure

echo ""
echo "=========================================================="
echo " [✓] eBPF Verifier Analysis Completed Successfully!"
echo "=========================================================="
