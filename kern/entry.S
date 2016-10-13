	/*
	 * just a entry for jumping to the init.c
	 */
	#include <inc/boot.h>
	#include <inc/mmu.h>
	#include <kern/multiboot1.h>

	#define HEADER_FLAGS	(MULTIBOOT_HEADER_PAGE_ALIGN | \
			 MULTIBOOT_HEADER_MEMORY_INFO | \
			 MULTIBOOT_HEADER_AOUT_KLUDGE)
	#define CHECKSUM	(-(MULTIBOOT_HEADER_MAGIC + HEADER_FLAGS))

	.text
	.code32
	.global _start
	 _start:
	 jmp multi_start

	 .global mboothdr
	 .align 4
		

	mboothdr:

	.long	MULTIBOOT_HEADER_MAGIC
	.long	HEADER_FLAGS
	.long	CHECKSUM

	.long	mboothdr	// header_addr: Address of the multiboot header
	.long	mboothdr	// load_addr: Start of the text segment
	.long	edata	// load_end_addr: End of the data segment
	.long	end // bss_end_addr: End of BSS segment
	.long	multi_start // entry_addr: Entrypoint
	.global eax, edx, esi, ebx
	eax:	
	.long 0x0
	edi:
	.long 0x0
	esi:
	.long 0x0
	ebx:
	.long 0x0

	.global multi_start, reload_gdt
	multi_start:
	movl %eax, eax
	movl %edi, edi
	movl %esi, esi
	movl %ebx, ebx

	lgdt gdtdesc_32
	ljmp $0x8, $reload_gdt
	
	.org 0x100
	reload_gdt:
	movw $0x10, %ax
	movw %ax, %ds
	movw %ax, %ss
	movw %ax, %gs
	movw %ax, %ss

	movl %edi, edi

	cmpw $0xdcba, 0x7200
	je  multi_boot
	
	cmpw $0xdcba, 0x7100
	je ap_boot

	tramp_mov:
	movl $0x40000, %edi
	movl $_binary_obj_boot_trampline_start, %esi
	movl $_binary_obj_boot_trampline_size, %ecx
	cld
	rep
	movsb
	
	ljmp $0x8, $0x40100
	

	//	cmpl %eax, 0x2badb002
	//	je multi_boot

	/*prepare for the paging*/

	multi_boot:
	cli
	cld


	movl $pml2, (pml1)
	addl $0x017, (pml1)

	movl $pml3, (pml2)
	addl $0x017, (pml2)

	movl $pml4, (pml3)
	addl $0x017, (pml3)

	movl $pml4, %eax
	movl $0x200, %ecx
	movl $0x017, %edi
	1:		  
	movl %edi, (%eax)
	addl $8, %eax
	addl $0x1000, %edi
	loop 1b

	ap_boot:
	movl %cr4, %eax
	orl $(0x20|0x80|0x200|0x400), %eax
	movl %eax, %cr4

	movl $pml4e, %eax
	movl %eax, %cr3

	movl $0xc0000080, %ecx
	rdmsr
	orl $(0x100|0x800|0x1000), %eax
	wrmsr

	lgdt gdtdesc_64

	movl %cr0, %eax
	orl $(0x1|0x80000000|0x40000|0x8), %eax
	movl %eax, %cr0

	ljmp $0x8, $long_m
	gdtdesc_32:
	.word 0x27
	.long gdt32
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
	.code64
	long_m:
	xorw %ax, %ax
	movw %ax, %gs
	movw %ax, %fs
	movw $0x10, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %ss

	movw $(0x0200 + '*'), 0xb8006


	to_real_kernel:
	/*	movq $i386_init, %r11
	call *%r11 */

	cmpw $0xdcba, 0x7100
	je ap_init_c

	movq $_stack_base, %rsp
	xorq %rdi, %rdi
	xorq %rsi, %rsi
	xorq %rdx, %rdx
	xorq %rcx, %rcx
	
	movl edi, %edi
	movl esi, %esi
	movl eax, %edx
	movl ebx, %ecx
	call i386_init

	ap_init_c:
	movq $_stack_base_ap, %rsp
	addq cpu_id, %rsp
	call ap_init

	.global gdtdesc_64
	gdtdesc_64:
	.word 0x17
	.long gdt64
	.long 0x0
	gdt64:
	DESCRIPTOR(0,0,0)
	DESCRIPTOR(0,0,0x298)
	DESCRIPTOR(0,0,0x292)
	.p2align 12
	_stack:
	.space 4096
	.globl _stack_base
	_stack_base:
	.space 4096
	.globl _stack_base_ap
	_stack_base_ap:
	.space 4096*33


	#define PMDS(START, ATTR, COUNT) 			\
	i = 0; 											\
	.rept (COUNT);  								\
	.quad (START) + (i << 21) + (ATTR); 	\
	i = i + 1;  									\
	.endr

	#define PDPE(START, ATTR, COUNT)   			\
	k = 0; 											\
	.rept (COUNT); 								\
	.quad (START)+(k << 12) + (ATTR); 		\
	k = k + 1; 										\
	.endr

		
	.data
	.p2align 12
	.global pml4e, pdpe, pde, pgt_end

	pml4e:
	.quad pdpe + 0x07
	.rept 511
	.quad 0x0
	.endr

	pdpe:
	PDPE(pde, 0x07, 512)

	pde:
	j = 0;
	.rept 512
	PMDS(0x0 + j<<30, 0x87, 512)
	j = j + 1;
	.endr
pgt_end:	

	.global npt
npt:		
	.rept (pgt_end - pml4e)
	.quad 0x0
	.endr


	.p2align 12
	.global pml1, pml2, pml3, pml4
	pml1:
	.rept 512
	.quad 0x0
	.endr
	.global pml2
	pml2:
	.rept 512
	.quad 0x0
	.endr
	.global pml3
	pml3:
	.rept 512
	.quad 0x0
	.endr
	.global pml4
	pml4:
	.rept 512
	.quad 0x0
	.endr
	.global pml_tmp2, pml_tmp3, pml_tmp4
	pml_tmp2:
	.rept 512
	.quad 0x0
	.endr
	pml_tmp3:
	.rept 512
	.quad 0x0
	.endr
	pml_tmp4:
	.rept 512
	.quad 0x0
	.endr

	.p2align 12
	.global t_bss, t_bss_end
t_bss:
	.space 4096 * 16
t_bss_end:	
	
	
