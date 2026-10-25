#!/usr/bin/env bash
set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$DIR"

echo "=========================================================="
echo " 0xLabs - Module 23: C++ Type Confusion & Unsafe Downcasts"
echo "=========================================================="

echo "[*] Step 1: Compiling binaries..."
make clean
make all

echo ""
echo "[*] Step 2: Testing Vulnerable static_cast downcasting..."
echo "----------------------------------------------------------"
./type_confusion_lab --vuln || true

echo ""
echo "[*] Step 3: Testing Secure dynamic_cast and Tagged Verification..."
echo "----------------------------------------------------------"
./type_confusion_lab --secure

echo ""
echo "[*] Step 4: Testing Sanitizer Runtime Detection (UBSan -fsanitize=vptr)..."
echo "----------------------------------------------------------"
echo "[*] Running type_confusion_ubsan --vuln (Expecting UBSan abort)..."
set +e
./type_confusion_ubsan --vuln
EXIT_CODE=$?
set -e
if [ $EXIT_CODE -ne 0 ]; then
    echo "[✓] UBSan successfully intercepted the downcast type confusion! (exit code $EXIT_CODE)"
else
    echo "[!] UBSan did not exit with error"
fi

echo ""
echo "=========================================================="
echo " [✓] Type Confusion Lab Successfully Executed!"
echo "=========================================================="
