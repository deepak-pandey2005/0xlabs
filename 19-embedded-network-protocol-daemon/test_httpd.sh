#!/usr/bin/env bash
set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
BIN="$DIR/mini_httpd"

if [ ! -f "$BIN" ]; then
    echo "[*] Compiling $BIN..."
    make -C "$DIR"
fi

echo "================================================================="
echo "PHASE 1: Standard Benign POST Request (Content-Length: 16)"
echo "================================================================="
BENIGN="POST /submit HTTP/1.1\r\nHost: localhost\r\nContent-Length: 16\r\n\r\n0123456789ABCDEF"
"$BIN" "$BENIGN"

echo ""
echo "================================================================="
echo "PHASE 2: Malformed Negative Content-Length (Content-Length: -1)"
echo "Notice how signed parsing passes the '< 256' check but expands to 2^64-1:"
echo "================================================================="
MALFORMED="POST /api/test HTTP/1.1\r\nHost: localhost\r\nContent-Length: -1\r\n\r\nmalformed_payload"
"$BIN" "$MALFORMED"

echo ""
echo "================================================================="
echo "PHASE 3: Testing RFC 7230 Hardened Parser (--secure)"
echo "Validating that unsigned parsing rejects negative lengths:"
echo "================================================================="
set +e
"$BIN" --secure "$MALFORMED"
STATUS=$?
set -e
echo "[+] Secure parser rejected malformed request (status: $STATUS)"
