#!/usr/bin/env bash
set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
BIN="$DIR/vtable_internals"

if [ ! -f "$BIN" ]; then
    echo "[*] Compiling vtable_internals..."
    make -C "$DIR"
fi

echo "================================================================="
echo "1. DEMANGLED VTABLE SYMBOLS IN BINARY (nm -C vtable_internals)"
echo "================================================================="
nm -C "$BIN" | grep -E "vtable for (BaseService|AdminService)"

echo ""
echo "================================================================="
echo "2. RUNTIME MEMORY LAYOUT & VPTR RESOLUTION"
echo "================================================================="
"$BIN"

echo ""
echo "================================================================="
echo "3. DISASSEMBLING DYNAMIC DISPATCH SEQUENCE IN GDB"
echo "Notice how the indirect call sequence loads [rdi] then calls [rax + 0x10]:"
echo "================================================================="
gdb -batch \
    -ex "set disassembly-flavor intel" \
    -ex "disassemble 'inspect_object(char const*, BaseService*)'" \
    "$BIN" | tail -n 16
