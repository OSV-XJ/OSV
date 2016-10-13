/*
 * this call bios to get the mem info, stored at the phy addr 0x8c00
 * set up the gdt table , jmp to the protet mode, setup simple page
 * table,then go to the mem.c
 */
#include<inc/boot.h>
#include<inc/mmu.h>
	.text
	.code16
.globl _start

_start:
	.byte 0xeb
	.byte start_of_setup-1f
1:
	.ascii "HdrS"
	.word	0x0204
	.globl realmode_swtch
realmode_swtch:
	.word 0, 0
start_sys_seg:
	.word 0
	.word 0
type_of_loader:
	.byte 0
loadflags:
	.byte 1
setup_move_size:
	.word 0x8000
code32_start:
	.long 0x100000
	
ramdisk_image:
	.long 0x0
ramdisk_size:
	.long 0
bootsect_kludge:
	.long 0
heap_end_ptr:
	.word 0
pad1:
	.word 0
	.globl cmd_line_ptr
cmd_line_ptr:
	.long 0
ramdisk_max:
	.long 0xffffffff
	/*
	 * let the kernel load into 0x100000
	 * keep the first 1MB empty
	 */
	.long 0x12345678
	.globl kernel_secotors
kernel_secotors:
	.long 0x0
	.globl ap_seg
ap_seg:
	.word 0x0
	ap_test:
	cli
	cld
	movw %cs, %ax
	movw %ax, %ds
	movw %ax, %es
	xorw %ax, %ax
	movw %ax, %fs
	movw $0xdcba, %fs:(0x7100)
	.byte 0xea
jmp_ap:
	.word clear
	.word 0x0

	.globl start_of_setup
start_of_setup:
	/*real setup code here*/
	cli
	cld
	movw %cs, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %ss
	movl kernel_secotors, %eax

	movw $0x700, %ax
	movw %ax, %fs
	xorl %edx, %edx
	xorl %eax, %eax
move_apcode:
	movw $(start_of_setup-ap_test), %cx
	movw $ap_test, %dx
	movw $0x0, %ax
2:
	movb %ds:(%edx), %bl 
	movb %bl, %fs:(%eax)
	inc %ax
	inc %dx
	loop 2b

	xorl %eax, %eax
	movw %cs, %fs:(jmp_ap-ap_test + 0x2)

	xorw %ax, %ax
	xorw %dx, %dx
	xorw %bx, %bx
	xorw %cx, %cx

	cli
	cld
	movw %cs, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %ss


	movw %cs, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %ss

	movw %ax, ap_seg


	xorl %edi, %edi
	andw $~3, %sp
	movzwl %sp, %esp
	movl $4, %esp
	pushw $0

	call csetup

	movw $0xb800, %ax
	movw %ax, %fs
	movw $(0x0200 + '*'), %fs:(0x00)

	xorl %edi, %edi
	andw $~3, %sp
	movzwl %sp, %esp
	movl $4, %esp
	pushw $0

	call absolute_adr
absolute_adr:
	popl %edi
	xorl %eax, %eax
	movw %cs, %ax
	shll $4, %eax
	addl %eax, %edi
	subl $absolute_adr, %edi

	movl %edi, %esi
	subl %eax, %esi /* now the %esi contains the offset in the %cs */

	movl $gdtaddr, %eax
	addl %esi, %eax
	addl %edi, (%eax)

	movl $gdtdesc_32, %eax
	addl %esi, %eax
	lgdtl (%eax)

	movl $jmp_addr, %eax
	addl %esi, %eax
	addl %edi, (%eax)

	xorw %ax, %ax
	movw %ax, %fs
	movw %ax, %gs
	movw %ax, %ss

	movl %edi, %esp


	/*prepare go to the protect mode*/
clear:
	xorw %ax, %ax
	movw %cs, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %ss
	
	xorw %ax, %ax
	movw %ax, %fs


	cmpw $0xdcba, %fs:(0x7100)
	jne bsp

	movl $gdtdesc_32, %eax
	
	lgdtl (%eax)

bsp:
	movl %cr0, %eax
	xorl $0x1, %eax
	movl %eax, %cr0

	.byte 0x66, 0xea
jmp_addr:
	.long protec_m
	.word 0x8

	.code32
	.global protec_m
protec_m:
	movw $0x10, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs
	movw %ax, %ss

	cmpw $0xdcba, 0x7100
	je ap_boot


	movl $tmp_stack_top, %esp
	addl %edi, %esp
	cmpw $0xabcd, 0x7d00
	jnz  to_kernel
	movl $kernel_secotors, %eax
	addl %edi, %eax
	pushl $0x88888888
	pushl (%eax)
	call bootmain


to_kernel:
	movw $(0x0200 + '*'), 0xb8004
	movl $sysx_info, %eax
	addl %edi, %eax
ap_boot:
	ljmp $0x8,$0x101000

	.align 16
	.global gdtdesc_32 
gdtdesc_32:		 
	.word 0x17
	.global gdtaddr
gdtaddr:
	.long gdt32

	.p2align 3
gdt32:
	SEG_NULL				# null seg
	SEG(STA_X|STA_R, 0x0, 0xffffffff)	# code seg
	SEG(STA_W, 0x0, 0xffffffff)	        # data seg
	.p2align 12
tmp_stack:
	.fill 1024, 1, 0
	.global tmp_stack_top
tmp_stack_top:

