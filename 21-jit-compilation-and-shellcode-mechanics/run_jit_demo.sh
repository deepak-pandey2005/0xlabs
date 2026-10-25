#!/usr/bin/env bash
set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
BIN="$DIR/jit_engine"

if [ ! -f "$BIN" ]; then
    echo "[*] Compiling $BIN..."
    make -C "$DIR"
fi

echo "================================================================="
echo "RUNNING JIT ENGINE WITH HARDWARE W^X MEMORY PROTECTION"
echo "================================================================="
"$BIN"
