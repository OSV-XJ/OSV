/* -*- linux-c -*- ------------------------------------------------------- *
 *
 *   Copyright (C) 1991, 1992 Linus Torvalds
 *   Copyright 2007 rPath, Inc. - All Rights Reserved
 *
 *   This file is part of the Linux kernel, and is made available under
 *   the terms of the GNU General Public License version 2.
 *
 * ----------------------------------------------------------------------- */

/*
 * arch/i386/boot/main.c
 *
 * Main module for the real-mode kernel code
 */
#include "code16gcc.h"
#include "boot.h"

struct boot_params boot_params __attribute__((aligned(16)));

char *HEAP = _end;
char *heap_end = _end;		/* Default end of heap = no heap */

#define OLD_CL_MAGIC 0xA33F
#define OLD_CL_ADDRESS 0x020
#define NEW_CL_POINTER 0x228
/*
 * Copy the header into the boot parameter block.  Since this
 * screws up the old-style command line protocol, adjust by
 * filling in the new-style command line pointer instead.
 */
/*
void copy_boot_params(void)
{
	struct old_cmdline {
		u16 cl_magic;
		u16 cl_offset;
	};
	const struct old_cmdline * const oldcmd =
		(const struct old_cmdline *)OLD_CL_ADDRESS;

	BUILD_BUG_ON(sizeof boot_params != 4096);
	memcpy(&boot_params.hdr, &hdr, sizeof hdr);

	if (!boot_params.hdr.cmd_line_ptr &&
	    oldcmd->cl_magic == OLD_CL_MAGIC) {
		// Old-style command line protocol. 
		u16 cmdline_seg;

		// Figure out if the command line falls in the region
		//   of memory that an old kernel would have copied up
		//   to 0x90000... 
		if (oldcmd->cl_offset < boot_params.hdr.setup_move_size)
			cmdline_seg = ds();
		else
			cmdline_seg = 0x9000;

		boot_params.hdr.cmd_line_ptr =
			(cmdline_seg << 4) + oldcmd->cl_offset;
	}
	}*/

/*
 * Set the keyboard repeat rate to maximum.  Unclear why this
 * is done here; this might be possible to kill off as stale code.
 */
void keyboard_set_repeat(void)
{
	u16 ax = 0x0305;
	u16 bx = 0;
	asm volatile("int $0x16"
		     : "+a" (ax), "+b" (bx)
		     : : "ecx", "edx", "esi", "edi");
}

/*
 * Get Intel SpeedStep (IST) information.
 */
void query_ist(void)
{
	asm("int $0x15"
	    : "=a" (boot_params.ist_info.signature),
	      "=b" (boot_params.ist_info.command),
	      "=c" (boot_params.ist_info.event),
	      "=d" (boot_params.ist_info.perf_level)
	    : "a" (0x0000e980),	 /* IST Support */
	      "d" (0x47534943)); /* Request value */
}

/*
 * Tell the BIOS what CPU mode we intend to run in.
 */
void set_bios_mode(void)
{
	u32 eax, ebx;

	eax = 0xec00;
	ebx = 2;
	asm volatile("int $0x15"
		     : "+a" (eax), "+b" (ebx)
		     : : "ecx", "edx", "esi", "edi");
}

void get_params(void)
{
	/* First, copy the boot header into the "zeropage" */
	//	copy_boot_params();

	/* End of heap check */
	
	/*	if (boot_params.hdr.loadflags & CAN_USE_HEAP) {
		heap_end = (char *)(boot_params.hdr.heap_end_ptr
				    +0x200-STACK_SIZE);
					 } */
	//
	//	heap_end = (char *)(0x8000);
	extern char svm_sig[];
	
	if(svm_sig[0] == '^')
	{
		printf("i'm guest\n");
		while(1);
	}
	
	for(uint16_t i = 0; i < sizeof(struct boot_params); i ++)
		((uint8_t *) &boot_params)[i] = 0;
	
	/* Tell the BIOS what CPU mode we intend to run in. */
	set_bios_mode();

	keyboard_set_repeat();
	
	/*Query e820map*/
	detect_memory();

	/*Query IST information*/
	query_ist();

	/* Set the video mode */
	set_video();
	
	/* Query MCA information */
	query_mca();

	/* Query APM information */
	query_apm_bios();

	/*Query voyager information*/
	query_voyager();
	
	/* Query EDD information */
	query_edd();
//	lock_cprintf("edd entries %d\n", boot_params.eddbuf_entries);
//	lock_cprintf("boot_params addr 0x%x\n", &boot_params);
//	lock_cprintf("e820 map entries is %d\n", boot_params.e820_entries);
}
