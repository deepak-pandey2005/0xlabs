#!/usr/bin/env bash
set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"

if [ ! -f "$DIR/app" ] || [ ! -f "$DIR/hook_strcmp.so" ]; then
    echo "[*] Building Module 10 targets..."
    make -C "$DIR"
fi

echo "================================================================="
echo "1. DYNAMIC RELOCATIONS INSPECTION (readelf -r app)"
echo "Notice how 'strcmp' is listed in the relocation table for .got.plt:"
echo "================================================================="
readelf -r "$DIR/app" | grep -E "Offset|strcmp|puts"

echo ""
echo "================================================================="
echo "2. NORMAL EXECUTION (Without Hook)"
echo "Passing guessed key 'test1234':"
echo "================================================================="
"$DIR/app" "test1234" || true

echo ""
echo "================================================================="
echo "3. HOOKED EXECUTION (Via LD_PRELOAD=./hook_strcmp.so)"
echo "Notice how our hook intercepts strcmp and reveals the real key!"
echo "================================================================="
LD_PRELOAD="$DIR/hook_strcmp.so" "$DIR/app" "test1234" || true

echo ""
echo "================================================================="
echo "4. AUTHENTICATING WITH RECOVERED KEY"
echo "================================================================="
"$DIR/app" "SUPER_SECRET_0xKEY_2026"
