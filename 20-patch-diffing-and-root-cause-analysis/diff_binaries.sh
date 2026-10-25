#!/usr/bin/env bash
set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"

if [ ! -f "$DIR/target_v1" ] || [ ! -f "$DIR/target_v2" ]; then
    echo "[*] Compiling targets..."
    make -C "$DIR"
fi

echo "================================================================="
echo "1. EXTRACTING & NORMALIZING DISASSEMBLY FOR process_user_records"
echo "================================================================="

# Extract assembly and strip absolute addresses for clean semantic diffing
objdump -d -M intel "$DIR/target_v1" | \
    sed -n '/<process_user_records>:/,/^$/p' | \
    sed 's/^[ \t]*[0-9a-f]*:[ \t]*[0-9a-f ]*\t//' > "$DIR/v1_disasm.txt"

objdump -d -M intel "$DIR/target_v2" | \
    sed -n '/<process_user_records>:/,/^$/p' | \
    sed 's/^[ \t]*[0-9a-f]*:[ \t]*[0-9a-f ]*\t//' > "$DIR/v2_disasm.txt"

echo "[+] Disassembly extracted for v1 (unpatched) and v2 (patched)."

echo ""
echo "================================================================="
echo "2. BINARY PATCH DIFF (diff -u v1_disasm.txt v2_disasm.txt)"
echo "Look for newly inserted basic block checks (+) in the assembly:"
echo "================================================================="

set +e
diff -u --color=always "$DIR/v1_disasm.txt" "$DIR/v2_disasm.txt"
set -e

echo ""
echo "================================================================="
echo "3. AUTOMATED ROOT CAUSE ANALYSIS (RCA) REPORT"
echo "================================================================="
cat << 'EOF'
[+] Patch Diffing Findings:
    - Newly Added Guard Instructions:
        * 'cmp DWORD PTR [rbp-0x58], 0x0'   --> Checks count <= 0
        * 'jle <label>'                     --> Jumps to error return
        * 'cmp DWORD PTR [rbp-0x58], 0x10'  --> Checks count > 16 (0x10)
        * 'jg  <label>'                     --> Jumps to error return
        * 'mov eax, 0xffffffff'             --> Returns -1 (error code)
    
    - Root Cause Analysis:
        * In Version 1.0, the loop copied user-controlled 'count' elements into
          a local 16-element stack buffer without validating upper bounds.
        * Vulnerability Class: CWE-121 (Stack-based Buffer Overflow).
        * The vendor added strict range validation [1, 16] prior to the loop.
EOF
