/*
 * This file must be the first one in the link order, 
 * so that mboothdr is the start of the text segment.
 */

#include <kern/multiboot1.h>

#define HEADER_FLAGS	(MULTIBOOT_HEADER_PAGE_ALIGN | \
			 MULTIBOOT_HEADER_MEMORY_INFO | \
			 MULTIBOOT_HEADER_AOUT_KLUDGE)
#define CHECKSUM	(-(MULTIBOOT_HEADER_MAGIC + HEADER_FLAGS))

.text
.align 4
.code32
	.global mboothdr
mboothdr:

.long	MULTIBOOT_HEADER_MAGIC
.long	HEADER_FLAGS
.long	CHECKSUM

.long	mboothdr	// header_addr: Address of the multiboot header
.long	mboothdr	// load_addr: Start of the text segment
.long	edata	// load_end_addr: End of the data segment
.long	end // bss_end_addr: End of BSS segment
.long	multi_start // entry_addr: Entrypoint

