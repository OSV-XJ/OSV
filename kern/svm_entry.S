#define VMRUN .byte 0x0F, 0x01, 0xD8
#define STGI  .byte 0x0F, 0x01, 0xDC
#define CLGI  .byte 0x0F, 0x01, 0xDD
#define VMSAVE .byte 0x0F, 0x01, 0xDB
#define VMLOAD .byte 0x0f, 0x01, 0xDA
	
	.text
	.code64
	.global svm_run, svm_guest_rest, svm_guest_save, vmcall
svm_run:
	mov %rdi, %rax
svm_guest_rest:
	CLGI
	pop %rdi
	pop %rsi
	pop %rcx
	pop %rdx
	pop %r8
	pop %r9
	pop %r10
	pop %r11
	pop %rbp
	pop %rbx
	pop %r12
	pop %r13
	pop %r14
	pop %r15
	pop %rax  #vmcb phy addr
	VMRUN

svm_guest_save:
	push %rax
	push %r15
	push %r14
	push %r13
	push %r12
	push %rbx
	push %rbp
	push %r11
	push %r10
	push %r9
	push %r8
	push %rdx
	push %rcx
	push %rsi
	push %rdi
movq %rsp, %rdi
	
	addq $1024*8, %rsp
	STGI
	
	call svm_exit_handler

	subq $1024*8, %rsp
	jmp svm_guest_rest


vmcall:
	push %rax
	mov %rdi, %rax
	mov %rsi, %rdi
	call *vm_call_table(, %rax, 8)
	pop %rax
	ret

