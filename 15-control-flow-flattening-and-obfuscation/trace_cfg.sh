#!/usr/bin/env bash
set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
BIN="$DIR/flattened_target"

if [ ! -f "$BIN" ]; then
    echo "[*] Compiling $BIN..."
    make -C "$DIR"
fi

echo "================================================================="
echo "1. DISASSEMBLY COMPARISON: Linear Flow vs Flattened Switch"
echo "================================================================="
echo "[*] Linear Function (compute_hash_original):"
objdump -d -M intel "$BIN" | grep -A 25 "<compute_hash_original>:" | head -n 25

echo ""
echo "[*] Flattened Function (compute_hash_flattened - Dispatcher Loop):"
objdump -d -M intel "$BIN" | grep -A 20 "<compute_hash_flattened>:" | head -n 20

echo ""
echo "================================================================="
echo "2. DYNAMIC CONTROL FLOW RECONSTRUCTION WITH GDB PYTHON"
echo "Recovering the true execution graph through runtime dispatcher logging:"
echo "================================================================="

gdb -q -batch \
    -ex "set pagination off" \
    -ex "set debuginfod enabled off" \
    -ex "file $BIN" \
    -ex "python
import gdb

class DispatcherTracker(gdb.Breakpoint):
    def stop(self):
        try:
            st = int(gdb.parse_and_eval('state'))
            names = {
                0x10: 'INIT (strlen & reset)',
                0x20: 'LOOP_CHECK (i < len)',
                0x30: 'LOOP_BODY (hash calculation)',
                0x40: 'BRANCH_CHECK (hash % 2)',
                0x50: 'EVEN_PATH (+42)',
                0x60: 'ODD_PATH (-13)',
                0x99: 'TERMINATE (return hash)'
            }
            desc = names.get(st, 'UNKNOWN')
            print(f'--> [CFG State: 0x{st:02x}] {desc}')
        except Exception as e:
            pass
        return False

gdb.execute('break compute_hash_flattened')
gdb.execute('run HI')
# Break at while loop dispatcher condition
DispatcherTracker('flattened_target.c:48')
gdb.execute('continue')
"
