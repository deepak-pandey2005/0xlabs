#!/usr/bin/env bash
set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
BIN="$DIR/anatomy"
BIN_STRIPPED="$DIR/anatomy_stripped"

if [ ! -f "$BIN" ]; then
    echo "[!] Compiling $BIN first..."
    make -C "$DIR"
fi

echo "================================================================="
echo "1. ELF HEADER (readelf -h anatomy)"
echo "================================================================="
readelf -h "$BIN" | head -n 14
echo "..."

echo ""
echo "================================================================="
echo "2. SECTION HEADERS (.text, .rodata, .data, .bss)"
echo "================================================================="
readelf -S "$BIN" | grep -E "Name|Type|Address|Offset|Size|Flags|\.text|\.rodata|\.data|\.bss"

echo ""
echo "================================================================="
echo "3. SIZE BREAKDOWN (size anatomy)"
echo "Notice how .bss does not occupy disk space, but size tracks it:"
echo "================================================================="
size "$BIN"

echo ""
echo "================================================================="
echo "4. SYMBOL TABLE (.symtab) vs STRIPPED BINARY"
echo "================================================================="
echo "Unstripped binary symbols for global variables and functions:"
nm "$BIN" | grep -E "print_memory_layout|g_initialized_counter|g_uninitialized_buffer|RO_VERSION_ID"
echo ""
echo "Stripped binary symbol check (notice symbols are removed):"
if nm "$BIN_STRIPPED" 2>&1 | grep -q "no symbols"; then
    echo "[✓] anatomy_stripped: 'no symbols' (symbol table removed for production/obfuscation)"
else
    echo "Stripped status:"
    nm "$BIN_STRIPPED" 2>&1 || true
fi

echo ""
echo "================================================================="
echo "5. RUNTIME MEMORY EXECUTION"
echo "================================================================="
"$BIN"
