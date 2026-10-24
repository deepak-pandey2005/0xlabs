#!/usr/bin/env bash
set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
BIN="$DIR/stack_frame"

if [ ! -f "$BIN" ]; then
    echo "[*] Building $BIN..."
    make -C "$DIR"
fi

echo "================================================================="
echo "Running GDB Automated Stack & Register Walkthrough"
echo "================================================================="
gdb -q -x "$DIR/gdb_stack_walk.gdb" "$BIN"
