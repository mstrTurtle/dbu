	.file	"a.cpp"
	.text
	.section	.rodata
	.type	_ZStL19piecewise_construct, @object
	.size	_ZStL19piecewise_construct, 1
_ZStL19piecewise_construct:
	.zero	1
	.align 8
	.type	_ZL23ACE_ONE_SECOND_IN_USECS, @object
	.size	_ZL23ACE_ONE_SECOND_IN_USECS, 8
_ZL23ACE_ONE_SECOND_IN_USECS:
	.quad	1000000
.LC0:
	.string	"a.cpp"
.LC1:
	.string	"sample info statement\n"
.LC2:
	.string	"sample debug statement\n"
.LC3:
	.string	"sample trace statement\n"
	.text
	.globl	main
	.type	main, @function
main:
.LFB2680:
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$48, %rsp
	call	_ZN11ACE_Log_Msg18last_error_adapterEv@PLT
	movl	%eax, -36(%rbp)
	call	_ZN11ACE_Log_Msg8instanceEv@PLT
	movq	%rax, -24(%rbp)
	movl	-36(%rbp), %edx
	movq	-24(%rbp), %rax
	movl	%edx, %r8d
	movl	$0, %ecx
	movl	$6, %edx
	leaq	.LC0(%rip), %rsi
	movq	%rax, %rdi
	call	_ZN11ACE_Log_Msg15conditional_setEPKciii@PLT
	movq	-24(%rbp), %rax
	leaq	.LC1(%rip), %rdx
	movl	$8, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	_ZN11ACE_Log_Msg3logE16ACE_Log_PriorityPKcz@PLT
	call	_ZN11ACE_Log_Msg18last_error_adapterEv@PLT
	movl	%eax, -32(%rbp)
	call	_ZN11ACE_Log_Msg8instanceEv@PLT
	movq	%rax, -16(%rbp)
	movl	-32(%rbp), %edx
	movq	-16(%rbp), %rax
	movl	%edx, %r8d
	movl	$0, %ecx
	movl	$7, %edx
	leaq	.LC0(%rip), %rsi
	movq	%rax, %rdi
	call	_ZN11ACE_Log_Msg15conditional_setEPKciii@PLT
	movq	-16(%rbp), %rax
	leaq	.LC2(%rip), %rdx
	movl	$4, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	_ZN11ACE_Log_Msg3logE16ACE_Log_PriorityPKcz@PLT
	call	_ZN11ACE_Log_Msg18last_error_adapterEv@PLT
	movl	%eax, -28(%rbp)
	call	_ZN11ACE_Log_Msg8instanceEv@PLT
	movq	%rax, -8(%rbp)
	movl	-28(%rbp), %edx
	movq	-8(%rbp), %rax
	movl	%edx, %r8d
	movl	$0, %ecx
	movl	$8, %edx
	leaq	.LC0(%rip), %rsi
	movq	%rax, %rdi
	call	_ZN11ACE_Log_Msg15conditional_setEPKciii@PLT
	movq	-8(%rbp), %rax
	leaq	.LC3(%rip), %rdx
	movl	$2, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	_ZN11ACE_Log_Msg3logE16ACE_Log_PriorityPKcz@PLT
	movl	$0, %eax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE2680:
	.size	main, .-main
	.ident	"GCC: (Ubuntu 9.4.0-1ubuntu1~20.04.1) 9.4.0"
	.section	.note.GNU-stack,"",@progbits
	.section	.note.gnu.property,"a"
	.align 8
	.long	 1f - 0f
	.long	 4f - 1f
	.long	 5
0:
	.string	 "GNU"
1:
	.align 8
	.long	 0xc0000002
	.long	 3f - 2f
2:
	.long	 0x3
3:
	.align 8
4:
