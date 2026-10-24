#!/usr/bin/env bash
set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
BIN="$DIR/rop_gadget_inspector"

if [ ! -f "$BIN" ]; then
    echo "[*] Building $BIN..."
    make -C "$DIR"
fi

echo "================================================================="
echo "1. TARGET FUNCTION SYMBOL ADDRESSES (Function Dispatch Table)"
echo "================================================================="
nm "$BIN" | grep -E "print_banner|process_argument|finalize_workflow"

echo ""
echo "================================================================="
echo "2. GADGET SCANNING (Instruction Sequences Ending in 'ret')"
echo "================================================================="
echo "Scanning .text for 'pop ... ; ret' and stack-manipulation gadgets:"
objdump -d -M intel "$BIN" | grep -B 2 -E "^\s*[0-9a-f]+:\s+c3\s+ret" | head -n 35

echo ""
echo "================================================================="
echo "3. INTEL CET IBT DETECTION ('endbr64' instructions)"
echo "Notice how modern GCC emits 'endbr64' (End Branch 64) for Indirect"
echo "Branch Tracking to defend against arbitrary indirect jumps:"
echo "================================================================="
objdump -d -M intel "$BIN" | grep -B 1 "endbr64" | head -n 12

echo ""
echo "================================================================="
echo "4. RUNTIME VERIFICATION"
echo "================================================================="
"$BIN"
