/* -*- linux-asm -*- */
	#include <inc/mmu.h>
	#include <inc/linuxdef.h>



	.code16
	.text
	.global ap_boot, svm_test, cr_test, to_real_svm, svm_sig

ap_boot:

	movw $0xdcba, 0x7100
para_pre:
	cli
	cld
	movw %cs, %ax
	movw %ax, %ds
	movw %ax, %es
	
	xorw %ax, %ax
	movw %ax, %fs
	lgdt gdtdesc_32


	movl %cr0, %eax
	orl $0x1, %eax
	cr_test:	
	movl %eax, %cr0

	.byte 0x66, 0xea
jmp_addr:
	.long (0x40000+protec_m)
	.word 0x8

	.code32
protec_m:
	movw $0x10, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs


	movl $boot_params, %edi
	addl $0x40000, %edi

	ljmp $0x8, $0x101100
	
	.code16

svm_jmp:
	cli
	movw %cs, %ax
	movw %ax, %ds
	movw %ax, %es
	lidt rm_idtr
ds1:
	lgdt  gdtsvm

	movl %cr0, %eax
	orl $0x1, %eax
	movl %eax, %cr0

svm_32_jmp:
	.byte 0x66, 0xea
	.long 0x40000+te
	.word 0x10

	.code32
	xorl %eax, %eax
te:
	movw $0x18, %ax
ds:
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs
	movw %ax, %ss
	movl $svm_sig, %ecx
	addl $0x40000, %ecx
	movl $0x2BADB002, %eax
	ljmp *(%ecx)

	.global gdtdesc_32, gdtaddr, gdt32, gdtsvm, gdtsvm_desc
	.align 16
gdtdesc_32:		 
	.word 0x27
gdtaddr:
	.long gdt32+0x40000

	.p2align 3
gdt32:
	SEG_NULL				# null seg
	SEG(STA_X|STA_R, 0x0, 0xffffffff)	# code seg
	SEG(STA_W, 0x0, 0xffffffff)	        # data seg
	/*16 bit code segment*/
	.long 0x0000ffff | ((0x40000 & 0x00ffff)<<16)
	.long 0x00009a00 | ((0x40000 & 0xff0000)>>16)
	/*16 bit data segment*/
	.long 0x0000ffff | ((0x40000 & 0x00ffff)<<16)
	.long 0x00009200 | ((0x40000 & 0xff0000)>>16)

	.p2align 4
gdtsvm:
	.word 0x1f
	.long gdtsvm_desc + 0x40000

	.p2align 4
gdtsvm_desc:
	SEG_NULL
	SEG_NULL
	SEG(STA_X|STA_R, 0x0, 0xffffffff)
	SEG(STA_W, 0x0, 0xffffffff)
	/*
	.long 0x0000ffff | ((0x0 & 0x00ffff)<<16)
	.long 0x00cf9b00 | ((0x0 & 0xff0000)>>16)
	.long 0x0000ffff | ((0x0 & 0x00ffff)<<16)
	.long 0x00cf9300 | ((0x0 & 0xff0000)>>16)
	*/

	.org 0xfa
svm_sig:	
	.long 0x0
	.word 0x10
	.code32
	.global trampoline_to_real
trampoline_to_real:


	movw $0xdcba, 0x7200
	movl $rm_idtr, %eax
	addl $0x40000, %eax
	lidtl (%eax)
	xorl %eax, %eax
	movw $0x20, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %gs
	movw %ax, %ss
	
	movl %cr0, %eax
	decl %eax

	ljmp $0x18, $t1
	
	.code16
	.global t1
t1:
	movl %eax, %cr0
to_real_svm:	

	ljmp $0x4000, $t2
	.global t2
t2:
	movw $0x4000, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %gs
	movw %ax, %ss
	movw %ax, %fs




	movw $0xfffc, %sp
	andw $~3, %sp
	movzwl %sp, %esp
	
	lidt rm_idtr
	sti


	call get_params

	ljmp $0x4000, $para_pre
ap_svm:
	hlt

	.align 8
	.global rm_idtr
rm_idtr:
	.word 256*4-1, 0, 0


