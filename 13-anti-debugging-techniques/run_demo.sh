#!/usr/bin/env bash
set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"

if [ ! -f "$DIR/anti_debug" ]; then
    echo "[*] Compiling anti_debug target..."
    make -C "$DIR"
fi

echo "================================================================="
echo "DEMO 1: Normal Clean Execution (No Debugger Attached)"
echo "================================================================="
"$DIR/anti_debug"

echo ""
echo "================================================================="
echo "DEMO 2: Running Under GDB (TracerPid Detection Traps Execution)"
echo "================================================================="
gdb -batch -ex "run" "$DIR/anti_debug" || true

echo ""
echo "================================================================="
echo "DEMO 3: Dynamic GDB Bypass of Anti-Debugging Checks"
echo "Setting breakpoints on check functions and overriding returns with 'return 0':"
echo "================================================================="
gdb -batch \
    -ex "break check_tracer_pid" \
    -ex "break check_ptrace_attached" \
    -ex "run" \
    -ex "return 0" \
    -ex "continue" \
    -ex "return 0" \
    -ex "continue" \
    "$DIR/anti_debug"
