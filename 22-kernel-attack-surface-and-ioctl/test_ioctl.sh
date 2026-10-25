#!/usr/bin/env bash
set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$DIR"

echo "=========================================================="
echo " 0xLabs - Module 22: Linux Kernel IOCTL Attack Surface"
echo "=========================================================="

echo "[*] Step 1: Compiling mock_kernel_ioctl..."
make clean
make all

echo ""
echo "[*] Step 2: Testing Direct User Pointer Access..."
echo "----------------------------------------------------------"
./mock_kernel_ioctl --direct

echo ""
echo "[*] Step 3: Testing Double-Fetch TOCTOU Flaw..."
echo "----------------------------------------------------------"
./mock_kernel_ioctl --double-fetch

echo ""
echo "[*] Step 4: Testing Secure Single-Fetch & Atomic Snapshot..."
echo "----------------------------------------------------------"
./mock_kernel_ioctl --secure

echo ""
echo "=========================================================="
echo " [✓] Kernel IOCTL Analysis & Boundary Test Completed!"
echo "=========================================================="
