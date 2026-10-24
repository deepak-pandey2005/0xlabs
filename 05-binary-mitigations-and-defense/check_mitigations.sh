#!/usr/bin/env bash
set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"

check_file() {
    local target="$1"
    local name="$(basename "$target")"
    printf "\n=== Checking Mitigations for: %-20s ===\n" "$name"

    # 1. Canary check
    if nm "$target" 2>/dev/null | grep -q "__stack_chk_fail"; then
        printf "  [*] Stack Canary : \033[32mFOUND\033[0m   (__stack_chk_fail present)\n"
    else
        printf "  [*] Stack Canary : \033[31mNONE\033[0m    (No canary symbol)\n"
    fi

    # 2. NX / DEP check
    if readelf -l "$target" | grep -A 1 "GNU_STACK" | grep -q "E"; then
        printf "  [*] NX / DEP     : \033[31mDISABLED\033[0m (Stack is Executable: RWE)\n"
    else
        printf "  [*] NX / DEP     : \033[32mENABLED\033[0m  (Stack is Non-Executable: RW)\n"
    fi

    # 3. PIE check
    local elf_type=$(readelf -h "$target" | grep "Type:" | awk '{print $2}')
    if [ "$elf_type" = "DYN" ]; then
        printf "  [*] PIE / ASLR   : \033[32mENABLED\033[0m  (Position Independent Executable: DYN)\n"
    else
        printf "  [*] PIE / ASLR   : \033[31mDISABLED\033[0m (Fixed Virtual Base: EXEC)\n"
    fi

    # 4. RELRO check
    local has_relro=false
    local has_bindnow=false

    if readelf -l "$target" | grep -q "GNU_RELRO"; then
        has_relro=true
    fi
    if readelf -d "$target" 2>/dev/null | grep -q "BIND_NOW"; then
        has_bindnow=true
    fi

    if $has_relro && $has_bindnow; then
        printf "  [*] RELRO        : \033[32mFULL RELRO\033[0m (GOT is read-only after load)\n"
    elif $has_relro; then
        printf "  [*] RELRO        : \033[33mPARTIAL RELRO\033[0m\n"
    else
        printf "  [*] RELRO        : \033[31mNO RELRO\033[0m\n"
    fi
}

echo "================================================================="
echo "0xLabs Binary Hardening & Mitigation Auditor"
echo "================================================================="

check_file "$DIR/demo_no_canary"
check_file "$DIR/demo_with_canary"
check_file "$DIR/demo_hardened"
check_file "$DIR/secure_refactoring"

echo ""
echo "================================================================="
echo "Runtime Canary Test (Overflowing demo_with_canary):"
echo "================================================================="
echo "Inputting 32 bytes into 16-byte buffer protected by Stack Canary:"
python3 -c 'import sys; sys.stdout.buffer.write(b"A"*32 + b"\n")' | "$DIR/demo_with_canary" || true
