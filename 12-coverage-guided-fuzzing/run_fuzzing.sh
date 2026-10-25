#!/usr/bin/env bash
set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"

if [ ! -f "$DIR/packet_fuzzer" ] || [ ! -f "$DIR/standalone_reproducer" ]; then
    echo "[*] Compiling LibFuzzer target with AddressSanitizer..."
    make -C "$DIR"
fi

# 1. Create a Seed Corpus directory
mkdir -p "$DIR/corpus"

# 2. Generate a valid seed packet:
# Magic: "PKT1"
# Type: 0x01 (TEXT)
# Length: 4 bytes
# Payload: "TEST"
python3 -c '
import sys
seed = b"PKT1" + bytes([0x01, 0x04]) + b"TEST"
with open("'$DIR'/corpus/seed_valid.bin", "wb") as f:
    f.write(seed)
'

echo "================================================================="
echo "1. SEED CORPUS INITIALIZED"
echo "================================================================="
xxd "$DIR/corpus/seed_valid.bin"

echo ""
echo "================================================================="
echo "2. LAUNCHING LIBFUZZER (Coverage-Guided Mutation Engine)"
echo "Targeting parse_packet() with AddressSanitizer coverage counters..."
echo "================================================================="

# Run fuzzer: stops automatically on crash or after 15 seconds
set +e
"$DIR/packet_fuzzer" "$DIR/corpus" -max_total_time=15 -workers=1
FUZZ_EXIT=$?
set -e

echo ""
echo "================================================================="
echo "3. CRASH ARTIFACT INVESTIGATION & TRIAGE"
echo "================================================================="
CRASH_FILE=$(find "$DIR" -maxdepth 1 -name "crash-*" | head -n 1)

if [ -n "$CRASH_FILE" ]; then
    echo "[+] Found generated crash artifact: $CRASH_FILE"
    echo "[*] Inspecting crash input bytes:"
    xxd "$CRASH_FILE"

    echo ""
    echo "================================================================="
    echo "4. REPRODUCING CRASH WITH STANDALONE REPRODUCER"
    echo "================================================================="
    set +e
    "$DIR/standalone_reproducer" "$CRASH_FILE"
    set -e
else
    echo "[-] No crash was generated within the time limit."
fi
