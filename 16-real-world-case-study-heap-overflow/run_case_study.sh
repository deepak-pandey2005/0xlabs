#!/usr/bin/env bash
set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
BIN="$DIR/cli_unescape"

if [ ! -f "$BIN" ]; then
    echo "[*] Compiling $BIN with AddressSanitizer..."
    make -C "$DIR"
fi

echo "================================================================="
echo "PHASE 1: Benign Input with Escaped Spaces ('normal\\ argument')"
echo "================================================================="
"$BIN" "normal\\ argument"

echo ""
echo "================================================================="
echo "PHASE 2: Malformed Input with Trailing Backslash ('vulnerable\\')"
echo "Notice how AddressSanitizer catches the null-byte leap heap overflow:"
echo "================================================================="
set +e
"$BIN" "vulnerable\\"
STATUS=$?
set -e
echo "[!] Process terminated with ASan status: $STATUS"

echo ""
echo "================================================================="
echo "PHASE 3: Secure Patched Implementation (--patch 'vulnerable\\')"
echo "Notice how the defensive check (*(src + 1) != '\\0') prevents the leak:"
echo "================================================================="
"$BIN" --patch "vulnerable\\"
