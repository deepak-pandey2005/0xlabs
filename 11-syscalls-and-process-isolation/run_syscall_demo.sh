#!/usr/bin/env bash
set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"

if [ ! -f "$DIR/syscall_demo" ] || [ ! -f "$DIR/seccomp_sandbox" ]; then
    echo "[*] Compiling Module 11 targets..."
    make -C "$DIR"
fi

echo "================================================================="
echo "DEMO 1: Direct Inline Assembly Syscalls (syscall instruction)"
echo "================================================================="
"$DIR/syscall_demo"

echo ""
echo "================================================================="
echo "DEMO 2: SECCOMP Sandbox (Permitted Syscalls Only)"
echo "================================================================="
"$DIR/seccomp_sandbox"

echo ""
echo "================================================================="
echo "DEMO 3: SECCOMP Sandbox Violation (Triggering Restricted Syscall)"
echo "Notice how the Linux kernel intercepts forbidden syscall and kills the process:"
echo "================================================================="
set +e
"$DIR/seccomp_sandbox" violate
STATUS=$?
echo "[!] Process terminated by Linux kernel with status: $STATUS"
