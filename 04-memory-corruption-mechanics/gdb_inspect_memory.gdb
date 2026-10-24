# GDB Script: Memory Corruption Inspection
set pagination off
set disassembly-flavor intel

echo \n--- [1] Breakpoint set before input ---\n
break vulnerable_buffer.c:36
run

echo \n--- [2] Inspecting Stack Layout Before Input ---\n
echo Address of buffer:\n
print &buffer
echo Address of is_admin:\n
print &is_admin
echo Value of is_admin:\n
print is_admin

echo \nSaved Frame Pointer ($rbp):\n
info frame

echo \nHex dump of current stack frame (20 words):\n
x/20wx $rsp

echo \nStep past fgets...\n
break vulnerable_buffer.c:42
continue

echo \n--- [3] Inspecting Stack Layout After Input ---\n
echo Value of is_admin now:\n
print/x is_admin
echo Notice how the memory locations were overwritten:\n
x/20wx $rsp

continue
quit
