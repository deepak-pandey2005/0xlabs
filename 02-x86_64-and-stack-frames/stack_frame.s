	.file	"stack_frame.c"
	.intel_syntax noprefix
	.text
.Ltext0:
	.file 0 "/home/deepak/OPENSOURCE/0xlabs/02-x86_64-and-stack-frames" "stack_frame.c"
	.section	.rodata
	.align 8
.LC0:
	.string	"[compute_sum] In function. Arg1=%d, Arg6=%d, Arg7(from stack)=%d\n"
	.align 8
.LC1:
	.string	"[compute_sum] Local var addresses: &local_temp=%p, &local_calc=%p\n"
	.text
	.globl	"compute_sum"
	.type	"compute_sum", @function
"compute_sum":
.LFB0:
	.file 1 "stack_frame.c"
	.loc 1 16 66
	.cfi_startproc
	push	rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	mov	rbp, rsp
	.cfi_def_cfa_register 6
	sub	rsp, 48
	mov	DWORD PTR -20[rbp], edi
	mov	DWORD PTR -24[rbp], esi
	mov	DWORD PTR -28[rbp], edx
	mov	DWORD PTR -32[rbp], ecx
	mov	DWORD PTR -36[rbp], r8d
	mov	DWORD PTR -40[rbp], r9d
	.loc 1 18 9
	mov	DWORD PTR -4[rbp], 4369
	.loc 1 19 24
	mov	edx, DWORD PTR -20[rbp]
	mov	eax, DWORD PTR -24[rbp]
	add	edx, eax
	.loc 1 19 28
	mov	eax, DWORD PTR -28[rbp]
	add	edx, eax
	.loc 1 19 32
	mov	eax, DWORD PTR -32[rbp]
	add	edx, eax
	.loc 1 19 36
	mov	eax, DWORD PTR -36[rbp]
	add	edx, eax
	.loc 1 19 40
	mov	eax, DWORD PTR -40[rbp]
	add	edx, eax
	.loc 1 19 44
	mov	eax, DWORD PTR 16[rbp]
	add	eax, edx
	.loc 1 19 9
	mov	DWORD PTR -8[rbp], eax
	.loc 1 21 5
	mov	ecx, DWORD PTR 16[rbp]
	mov	edx, DWORD PTR -40[rbp]
	mov	eax, DWORD PTR -20[rbp]
	lea	rdi, .LC0[rip]
	mov	esi, eax
	mov	eax, 0
	call	"printf"@PLT
	.loc 1 22 5
	lea	rdx, -8[rbp]
	lea	rax, -4[rbp]
	lea	rcx, .LC1[rip]
	mov	rsi, rax
	mov	rdi, rcx
	mov	eax, 0
	call	"printf"@PLT
	.loc 1 25 12
	mov	eax, DWORD PTR -8[rbp]
	.loc 1 26 1
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE0:
	.size	"compute_sum", .-"compute_sum"
	.section	.rodata
	.align 8
.LC2:
	.string	"[caller_function] Calling compute_sum with 7 arguments..."
	.align 8
.LC3:
	.string	"[caller_function] Result returned: %d (x=%d, y=%d)\n"
	.text
	.globl	"caller_function"
	.type	"caller_function", @function
"caller_function":
.LFB1:
	.loc 1 29 28
	.cfi_startproc
	push	rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	mov	rbp, rsp
	.cfi_def_cfa_register 6
	sub	rsp, 16
	.loc 1 30 9
	mov	DWORD PTR -4[rbp], 10
	.loc 1 31 9
	mov	DWORD PTR -8[rbp], 20
	.loc 1 33 5
	lea	rax, .LC2[rip]
	mov	rdi, rax
	call	"puts"@PLT
	.loc 1 35 18
	sub	rsp, 8
	push	7
	mov	r9d, 6
	mov	r8d, 5
	mov	ecx, 4
	mov	edx, 3
	mov	esi, 2
	mov	edi, 1
	call	"compute_sum"
	add	rsp, 16
	mov	DWORD PTR -12[rbp], eax
	.loc 1 37 5
	mov	ecx, DWORD PTR -8[rbp]
	mov	edx, DWORD PTR -4[rbp]
	mov	eax, DWORD PTR -12[rbp]
	lea	rdi, .LC3[rip]
	mov	esi, eax
	mov	eax, 0
	call	"printf"@PLT
	.loc 1 38 1
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE1:
	.size	"caller_function", .-"caller_function"
	.section	.rodata
	.align 8
.LC4:
	.string	"=== 0xLabs: x86_64 Stack Frame & Calling Convention Demo ==="
	.text
	.globl	"main"
	.type	"main", @function
"main":
.LFB2:
	.loc 1 40 16
	.cfi_startproc
	push	rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	mov	rbp, rsp
	.cfi_def_cfa_register 6
	.loc 1 41 5
	lea	rax, .LC4[rip]
	mov	rdi, rax
	call	"puts"@PLT
	.loc 1 42 5
	call	"caller_function"
	.loc 1 43 12
	mov	eax, 0
	.loc 1 44 1
	pop	rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE2:
	.size	"main", .-"main"
.Letext0:
	.file 2 "/usr/include/stdio.h"
	.section	.debug_info,"",@progbits
.Ldebug_info0:
	.long	0x178
	.value	0x5
	.byte	0x1
	.byte	0x8
	.long	.Ldebug_abbrev0
	.uleb128 0x5
	.long	.LASF15
	.byte	0x1d
	.byte	0x3
	.long	0x31647
	.long	.LASF0
	.long	.LASF1
	.quad	.Ltext0
	.quad	.Letext0-.Ltext0
	.long	.Ldebug_line0
	.uleb128 0x1
	.byte	0x8
	.byte	0x7
	.long	.LASF2
	.uleb128 0x1
	.byte	0x4
	.byte	0x7
	.long	.LASF3
	.uleb128 0x1
	.byte	0x1
	.byte	0x8
	.long	.LASF4
	.uleb128 0x1
	.byte	0x2
	.byte	0x7
	.long	.LASF5
	.uleb128 0x1
	.byte	0x1
	.byte	0x6
	.long	.LASF6
	.uleb128 0x1
	.byte	0x2
	.byte	0x5
	.long	.LASF7
	.uleb128 0x6
	.byte	0x4
	.byte	0x5
	.string	"int"
	.uleb128 0x1
	.byte	0x8
	.byte	0x5
	.long	.LASF8
	.uleb128 0x1
	.byte	0x1
	.byte	0x6
	.long	.LASF9
	.uleb128 0x7
	.long	0x6b
	.uleb128 0x8
	.byte	0x8
	.long	0x72
	.uleb128 0x9
	.long	.LASF16
	.byte	0x2
	.value	0x172
	.byte	0xc
	.long	0x5d
	.long	0x95
	.uleb128 0xa
	.long	0x77
	.uleb128 0xb
	.byte	0
	.uleb128 0xc
	.long	.LASF17
	.byte	0x1
	.byte	0x28
	.byte	0x5
	.long	0x5d
	.quad	.LFB2
	.quad	.LFE2-.LFB2
	.uleb128 0x1
	.byte	0x9c
	.uleb128 0xd
	.long	.LASF11
	.byte	0x1
	.byte	0x1d
	.byte	0x6
	.quad	.LFB1
	.quad	.LFE1-.LFB1
	.uleb128 0x1
	.byte	0x9c
	.long	0xf5
	.uleb128 0x4
	.string	"x"
	.byte	0x1e
	.long	0x5d
	.uleb128 0x2
	.byte	0x91
	.sleb128 -20
	.uleb128 0x4
	.string	"y"
	.byte	0x1f
	.long	0x5d
	.uleb128 0x2
	.byte	0x91
	.sleb128 -24
	.uleb128 0x3
	.long	.LASF10
	.byte	0x23
	.long	0x5d
	.uleb128 0x2
	.byte	0x91
	.sleb128 -28
	.byte	0
	.uleb128 0xe
	.long	.LASF12
	.byte	0x1
	.byte	0x10
	.byte	0x5
	.long	0x5d
	.quad	.LFB0
	.quad	.LFE0-.LFB0
	.uleb128 0x1
	.byte	0x9c
	.uleb128 0x2
	.string	"a"
	.byte	0x15
	.long	0x5d
	.uleb128 0x2
	.byte	0x91
	.sleb128 -36
	.uleb128 0x2
	.string	"b"
	.byte	0x1c
	.long	0x5d
	.uleb128 0x2
	.byte	0x91
	.sleb128 -40
	.uleb128 0x2
	.string	"c"
	.byte	0x23
	.long	0x5d
	.uleb128 0x2
	.byte	0x91
	.sleb128 -44
	.uleb128 0x2
	.string	"d"
	.byte	0x2a
	.long	0x5d
	.uleb128 0x2
	.byte	0x91
	.sleb128 -48
	.uleb128 0x2
	.string	"e"
	.byte	0x31
	.long	0x5d
	.uleb128 0x2
	.byte	0x91
	.sleb128 -52
	.uleb128 0x2
	.string	"f"
	.byte	0x38
	.long	0x5d
	.uleb128 0x2
	.byte	0x91
	.sleb128 -56
	.uleb128 0x2
	.string	"g"
	.byte	0x3f
	.long	0x5d
	.uleb128 0x2
	.byte	0x91
	.sleb128 0
	.uleb128 0x3
	.long	.LASF13
	.byte	0x12
	.long	0x5d
	.uleb128 0x2
	.byte	0x91
	.sleb128 -20
	.uleb128 0x3
	.long	.LASF14
	.byte	0x13
	.long	0x5d
	.uleb128 0x2
	.byte	0x91
	.sleb128 -24
	.byte	0
	.byte	0
	.section	.debug_abbrev,"",@progbits
.Ldebug_abbrev0:
	.uleb128 0x1
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.byte	0
	.byte	0
	.uleb128 0x2
	.uleb128 0x5
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0x21
	.sleb128 1
	.uleb128 0x3b
	.uleb128 0x21
	.sleb128 16
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0x3
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0x21
	.sleb128 1
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0x21
	.sleb128 9
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0x4
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0x21
	.sleb128 1
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0x21
	.sleb128 9
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0x5
	.uleb128 0x11
	.byte	0x1
	.uleb128 0x25
	.uleb128 0xe
	.uleb128 0x13
	.uleb128 0xb
	.uleb128 0x90
	.uleb128 0xb
	.uleb128 0x91
	.uleb128 0x6
	.uleb128 0x3
	.uleb128 0x1f
	.uleb128 0x1b
	.uleb128 0x1f
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x7
	.uleb128 0x10
	.uleb128 0x17
	.byte	0
	.byte	0
	.uleb128 0x6
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0x8
	.byte	0
	.byte	0
	.uleb128 0x7
	.uleb128 0x26
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x8
	.uleb128 0xf
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x9
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3c
	.uleb128 0x19
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xa
	.uleb128 0x5
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xb
	.uleb128 0x18
	.byte	0
	.byte	0
	.byte	0
	.uleb128 0xc
	.uleb128 0x2e
	.byte	0
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x7
	.uleb128 0x40
	.uleb128 0x18
	.uleb128 0x7c
	.uleb128 0x19
	.byte	0
	.byte	0
	.uleb128 0xd
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x7
	.uleb128 0x40
	.uleb128 0x18
	.uleb128 0x7c
	.uleb128 0x19
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xe
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x7
	.uleb128 0x40
	.uleb128 0x18
	.uleb128 0x7c
	.uleb128 0x19
	.byte	0
	.byte	0
	.byte	0
	.section	.debug_aranges,"",@progbits
	.long	0x2c
	.value	0x2
	.long	.Ldebug_info0
	.byte	0x8
	.byte	0
	.value	0
	.value	0
	.quad	.Ltext0
	.quad	.Letext0-.Ltext0
	.quad	0
	.quad	0
	.section	.debug_line,"",@progbits
.Ldebug_line0:
	.section	.debug_str,"MS",@progbits,1
.LASF5:
	.string	"short unsigned int"
.LASF14:
	.string	"local_calc"
.LASF3:
	.string	"unsigned int"
.LASF12:
	.string	"compute_sum"
.LASF2:
	.string	"long unsigned int"
.LASF9:
	.string	"char"
.LASF11:
	.string	"caller_function"
.LASF4:
	.string	"unsigned char"
.LASF17:
	.string	"main"
.LASF10:
	.string	"result"
.LASF8:
	.string	"long int"
.LASF16:
	.string	"printf"
.LASF7:
	.string	"short int"
.LASF13:
	.string	"local_temp"
.LASF6:
	.string	"signed char"
.LASF15:
	.string	"GNU C23 16.2.1 20260810 -masm=intel -mtune=generic -march=x86-64 -g -O0 -fno-omit-frame-pointer -fno-stack-protector"
	.section	.debug_line_str,"MS",@progbits,1
.LASF1:
	.string	"/home/deepak/OPENSOURCE/0xlabs/02-x86_64-and-stack-frames"
.LASF0:
	.string	"stack_frame.c"
	.ident	"GCC: (GNU) 16.2.1 20260810"
	.section	.note.GNU-stack,"",@progbits
