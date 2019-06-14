	.file	"wc209.c"
	.section	.rodata
	.align 8
.LC0:
	.string	"Error: line %d: unterminated comment\n"
.LC1:
	.string	"%d %d %d\n"
	.text
	.globl	main
	.type	main, @function
main:
.LFB0:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movl	$0, -24(%rbp)
	movl	$0, -20(%rbp)
	movl	$0, -16(%rbp)
	movl	$1, -12(%rbp)
	movl	$0, -8(%rbp)
	jmp	.L2
.L40:
	cmpl	$6, -8(%rbp)
	ja	.L44
	movl	-8(%rbp), %eax
	movq	.L5(,%rax,8), %rax
	jmp	*%rax
	.section	.rodata
	.align 8
	.align 4
.L5:
	.quad	.L4
	.quad	.L6
	.quad	.L7
	.quad	.L8
	.quad	.L9
	.quad	.L10
	.quad	.L11
	.text
.L4:
	call	__ctype_b_loc
	movq	(%rax), %rax
	movl	-4(%rbp), %edx
	movslq	%edx, %rdx
	addq	%rdx, %rdx
	addq	%rdx, %rax
	movzwl	(%rax), %eax
	movzwl	%ax, %eax
	andl	$8192, %eax
	testl	%eax, %eax
	je	.L12
	movl	$1, -8(%rbp)
	cmpl	$10, -4(%rbp)
	jne	.L13
	addl	$1, -20(%rbp)
.L13:
	addl	$1, -16(%rbp)
	jmp	.L14
.L12:
	cmpl	$47, -4(%rbp)
	jne	.L15
	movl	$2, -8(%rbp)
	jmp	.L16
.L15:
	movl	$3, -8(%rbp)
.L16:
	addl	$1, -24(%rbp)
	addl	$1, -16(%rbp)
.L14:
	addl	$1, -20(%rbp)
	jmp	.L2
.L6:
	call	__ctype_b_loc
	movq	(%rax), %rax
	movl	-4(%rbp), %edx
	movslq	%edx, %rdx
	addq	%rdx, %rdx
	addq	%rdx, %rax
	movzwl	(%rax), %eax
	movzwl	%ax, %eax
	andl	$8192, %eax
	testl	%eax, %eax
	je	.L17
	cmpl	$10, -4(%rbp)
	jne	.L18
	addl	$1, -20(%rbp)
.L18:
	addl	$1, -16(%rbp)
	jmp	.L2
.L17:
	cmpl	$47, -4(%rbp)
	jne	.L20
	movl	$2, -8(%rbp)
	jmp	.L21
.L20:
	movl	$3, -8(%rbp)
.L21:
	addl	$1, -24(%rbp)
	addl	$1, -16(%rbp)
	jmp	.L2
.L7:
	call	__ctype_b_loc
	movq	(%rax), %rax
	movl	-4(%rbp), %edx
	movslq	%edx, %rdx
	addq	%rdx, %rdx
	addq	%rdx, %rax
	movzwl	(%rax), %eax
	movzwl	%ax, %eax
	andl	$8192, %eax
	testl	%eax, %eax
	je	.L22
	movl	$1, -8(%rbp)
	cmpl	$10, -4(%rbp)
	jne	.L23
	addl	$1, -20(%rbp)
.L23:
	addl	$1, -16(%rbp)
	jmp	.L2
.L22:
	cmpl	$42, -4(%rbp)
	jne	.L25
	movl	$5, -8(%rbp)
	subl	$1, -24(%rbp)
	subl	$1, -16(%rbp)
	movl	-20(%rbp), %eax
	movl	%eax, -12(%rbp)
	jmp	.L2
.L25:
	cmpl	$47, -4(%rbp)
	je	.L26
	movl	$3, -8(%rbp)
.L26:
	addl	$1, -16(%rbp)
	jmp	.L2
.L8:
	call	__ctype_b_loc
	movq	(%rax), %rax
	movl	-4(%rbp), %edx
	movslq	%edx, %rdx
	addq	%rdx, %rdx
	addq	%rdx, %rax
	movzwl	(%rax), %eax
	movzwl	%ax, %eax
	andl	$8192, %eax
	testl	%eax, %eax
	je	.L27
	movl	$1, -8(%rbp)
	cmpl	$10, -4(%rbp)
	jne	.L28
	addl	$1, -20(%rbp)
.L28:
	addl	$1, -16(%rbp)
	jmp	.L2
.L27:
	cmpl	$47, -4(%rbp)
	jne	.L30
	movl	$4, -8(%rbp)
.L30:
	addl	$1, -16(%rbp)
	jmp	.L2
.L9:
	call	__ctype_b_loc
	movq	(%rax), %rax
	movl	-4(%rbp), %edx
	movslq	%edx, %rdx
	addq	%rdx, %rdx
	addq	%rdx, %rax
	movzwl	(%rax), %eax
	movzwl	%ax, %eax
	andl	$8192, %eax
	testl	%eax, %eax
	je	.L31
	movl	$1, -8(%rbp)
	cmpl	$10, -4(%rbp)
	jne	.L32
	addl	$1, -20(%rbp)
.L32:
	addl	$1, -16(%rbp)
	jmp	.L2
.L31:
	cmpl	$42, -4(%rbp)
	jne	.L34
	movl	$5, -8(%rbp)
	subl	$1, -16(%rbp)
	movl	-20(%rbp), %eax
	movl	%eax, -12(%rbp)
	jmp	.L2
.L34:
	cmpl	$47, -4(%rbp)
	je	.L35
	movl	$3, -8(%rbp)
.L35:
	addl	$1, -16(%rbp)
	jmp	.L2
.L10:
	cmpl	$10, -4(%rbp)
	jne	.L36
	addl	$1, -20(%rbp)
	addl	$1, -16(%rbp)
	jmp	.L2
.L36:
	cmpl	$42, -4(%rbp)
	jne	.L2
	movl	$6, -8(%rbp)
	jmp	.L2
.L11:
	cmpl	$47, -4(%rbp)
	jne	.L38
	movl	$1, -8(%rbp)
	addl	$1, -16(%rbp)
	jmp	.L2
.L38:
	cmpl	$42, -4(%rbp)
	je	.L2
	movl	$5, -8(%rbp)
	cmpl	$10, -4(%rbp)
	jne	.L2
	addl	$1, -20(%rbp)
	addl	$1, -16(%rbp)
	jmp	.L2
.L44:
	nop
.L2:
	call	getchar
	movl	%eax, -4(%rbp)
	cmpl	$-1, -4(%rbp)
	jne	.L40
	cmpl	$5, -8(%rbp)
	je	.L41
	cmpl	$6, -8(%rbp)
	jne	.L42
.L41:
	movq	stderr(%rip), %rax
	movl	-12(%rbp), %edx
	movl	$.LC0, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	fprintf
	movl	$1, %eax
	jmp	.L43
.L42:
	movl	-16(%rbp), %ecx
	movl	-24(%rbp), %edx
	movl	-20(%rbp), %eax
	movl	%eax, %esi
	movl	$.LC1, %edi
	movl	$0, %eax
	call	printf
	movl	$0, %eax
.L43:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE0:
	.size	main, .-main
	.ident	"GCC: (Ubuntu 5.4.0-6ubuntu1~16.04.9) 5.4.0 20160609"
	.section	.note.GNU-stack,"",@progbits
