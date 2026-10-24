# GDB Script: Automated Stack Frame & Register Walkthrough
set pagination off
set disassembly-flavor intel

echo \n--- [1] Setting Breakpoint at compute_sum ---\n
break compute_sum
run

echo \n--- [2] Entering compute_sum (Prologue Inspection) ---\n
echo Disassembly of compute_sum:\n
disassemble compute_sum

echo \nRegisters on Entry:\n
info registers rdi rsi rdx rcx r8 r9 rsp rbp rip

echo \nNotice:\n
echo   rdi (arg 1) = 1\n
echo   rsi (arg 2) = 2\n
echo   rdx (arg 3) = 3\n
echo   rcx (arg 4) = 4\n
echo   r8  (arg 5) = 5\n
echo   r9  (arg 6) = 6\n
echo   Arg 7 was passed on the stack!\n

echo \nStack Memory Dump (top 8 quadwords at $rsp):\n
x/8gx $rsp

echo \nNotice the top of the stack ($rsp) holds the return address pointing back to caller_function!\n
info symbol *(void**)$rsp

echo \nStep past the prologue (push rbp; mov rbp, rsp; sub rsp, ...)\n
nexti
nexti
nexti

echo \nRegisters after prologue:\n
info registers rsp rbp rip

echo \nContinue to return:\n
finish

echo \n--- [3] Back in caller_function ---\n
echo Return value placed in RAX:\n
info registers rax

quit
