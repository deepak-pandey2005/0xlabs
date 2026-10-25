#!/usr/bin/env bash
set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$DIR"

echo "=========================================================="
echo " 0xLabs - Module 25: Kernel Heap & SLUB Use-After-Free"
echo "=========================================================="

echo "[*] Step 1: Compiling kernel_slub_mock..."
make clean
make all

echo ""
echo "[*] Step 2: Testing Kernel Use-After-Free & Object Confusion..."
echo "----------------------------------------------------------"
./kernel_slub_mock --uaf

echo ""
echo "[*] Step 3: Inspecting CONFIG_SLAB_FREELIST_HARDENED..."
echo "----------------------------------------------------------"
./kernel_slub_mock --hardened

echo ""
echo "=========================================================="
echo " [✓] Kernel SLUB Lab Verification Complete!"
echo "=========================================================="
