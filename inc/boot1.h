#ifndef JOS_MACHINE_BOOT_H
#define JOS_MACHINE_BOOT_H

#include <inc/e820.h>

#define DIRECT_BOOT_EAX_MAGIC	0x6A6F7362
#define	SYSXBOOT_EAX_MAGIC	0x910DFAA0

#ifndef __ASSEMBLER__

struct sysx_info {
    uint32_t extmem_kb;
    uint32_t cmdline;
    struct e820entry e820_map[E820MAX];
    uint8_t e820_nents;
} __attribute__ ((packed));

// Physical address to load the bootstrap code for
// application processors.  See boot/bootother.S.
#define APBOOTSTRAP 0x7000

void start_ap(void);

#endif

#endif
