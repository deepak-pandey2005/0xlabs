#!/usr/bin/env bash
set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
BIN="$DIR/toctou_demo"

if [ ! -f "$BIN" ]; then
    echo "[*] Compiling $BIN..."
    make -C "$DIR"
fi

TARGET="$DIR/target_link"
PROTECTED="$DIR/protected_file.txt"

cleanup() {
    rm -f "$TARGET" "$PROTECTED"
}
trap cleanup EXIT

echo "================================================================="
echo "PHASE 1: Setting Up Target Environment"
echo "================================================================="
echo "ORIGINAL_SECRET_DATABASE_CONFIG" > "$PROTECTED"
echo "[+] Created sensitive protected file: protected_file.txt"
cat "$PROTECTED"

echo ""
echo "================================================================="
echo "PHASE 2: Simulating TOCTOU Race Attack Against Vulnerable Binary"
echo "Target: Swapping target_link to point to protected_file during race window"
echo "================================================================="

# Start vulnerable program with a 400ms delay between check and open
"$BIN" --vuln "$TARGET" 400000 &
PID=$!

# Sleep briefly (100ms) to ensure access() check has already executed
sleep 0.1

# Perform the race symlink swap
ln -s "$PROTECTED" "$TARGET"
echo "[!] Attacker created symlink during race window: target_link -> protected_file.txt"

# Wait for vulnerable process to finish
wait $PID || true

echo ""
echo "[*] Checking protected_file.txt contents:"
cat "$PROTECTED"
if grep -q "CRITICAL_PAYLOAD_DATA" "$PROTECTED"; then
    echo "[!] TOCTOU EXPLOITATION CONFIRMED: protected_file was overwritten through the symlink!"
fi

echo ""
echo "================================================================="
echo "PHASE 3: Testing Secure Atomic Implementation"
echo "Testing with O_NOFOLLOW | O_EXCL flags on the symlink target:"
echo "================================================================="
echo "ORIGINAL_SECRET_DATABASE_CONFIG" > "$PROTECTED"

# Attempting to write through symlink using secure open()
set +e
"$BIN" --secure "$TARGET"
set -e

echo ""
echo "[*] Checking protected_file.txt contents after secure execution:"
cat "$PROTECTED"
if grep -q "ORIGINAL_SECRET_DATABASE_CONFIG" "$PROTECTED"; then
    echo "[✓] DEFENSE VERIFIED: Sensitive file remained untouched. Atomic flags rejected symlink."
fi
