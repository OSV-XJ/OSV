#ifndef JOS_MACHINE_E820_H
#define JOS_MACHINE_E820_H

#ifndef  __ASSEMBLER__

#include <inc/types.h>

#define E820_RAM	1
#define E820_RESERVED	2
#define E820_ACPI	3
#define E820_NVS	4

#define E820MAX	128		/* number of entries in sysx_info.e820_map */

#define SMAP	0x534d4150	/* ASCII "SMAP" */

struct e820entry {
	uint64_t addr;	/* start of memory segment */
	uint64_t size;	/* size of memory segment */
	uint32_t type;	/* type of memory segment */
} __attribute__((packed));

struct e820map
{
	uint32_t nr_map;
	struct e820entry map[E820MAX];
};

#endif

#endif
