	.global setup
setup:  
	.text
	.code16
	cli
	cld

	xorw %ax, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %ss
	movw $0xb800, %ax
     movw %ax, %fs
	  movw $(0x0200 + '*'), %fs:(0x02)



	/*reset the disk controller(copy from the Linux)*/
	// movw $0x0000, %ax
	// movb $0x80, %dl
	//int $0x13
	/* copy all the kernel image to the memory
	 * in case there is no boot loader.
* assumtion: kernel is from the 2nd sectors, and about 8
* sectors(4kbytes). loaded at the address :8c00
*/
	movw $0x8000, %bx

	movw $0x0213, %ax

	movw $0x0002, %cx
	movw $0x0080, %dx

	int $0x13
	

	cmpb $0x0, %ah
	jne error
	seta20.1:
  inb     $0x64,%al               # Wait for not busy
  testb   $0x2,%al
  jnz     seta20.1

  movb    $0xd1,%al               # 0xd1 -> port 0x64
  outb    %al,$0x64

seta20.2:
  inb     $0x64,%al               # Wait for not busy
  testb   $0x2,%al
  jnz     seta20.2

  movb    $0xdf,%al               # 0xdf -> port 0x60
  outb    %al,$0x60

to_setup:
	movw $0xabcd, 0x7d00 
	ljmp $0x800, $0x0
	/*should never return here, in case return to the bios*/
error:
	ljmp $0xf000, $0xfff0


	.org 0x100
	.word 0x0
	.org 0x1f0
pad:
	.byte 0
setup_sects:
	.byte 0xb
root_flags:
	.word 0
syssize:
	.long 0
ram_size:
	.word 0
vid_mode:
	.word 0
root_dev:
	.word 0
boot_flags:		 
	.word 0xAA55
