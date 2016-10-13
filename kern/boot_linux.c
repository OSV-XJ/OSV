#include <inc/types.h>
#include <inc/linux-header.h>
#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/x86.h>


#define SECTSIZE 512

static void
waitdisk(void)
{
    // wait for disk reaady
    while ((inb(0x1F7) & 0xC0) != 0x40)
	/* do nothing */ ;
}

static void
readsect(void *dst, uint32_t offset)
{
    // wait for disk to be ready
    //waitdisk();

    outb(0x1F2, 1);		// count = 1
    outb(0x1F3, offset);
    outb(0x1F4, offset >> 8);
    outb(0x1F5, offset >> 16);
    outb(0x1F6, (offset >> 24) | 0xE0);
    outb(0x1F7, 0x20);		// cmd 0x20 - read sectors

    // wait for disk to be ready
   // waitdisk();

    // read a sector
    insl(0x1F0, dst, SECTSIZE / 4);
}

// Read 'count' bytes at 'offset' from kernel into virtual address 'va'.
// Might copy more than asked
void
readseg(uint64_t va, uint32_t count, uint32_t offset)
{
    uint64_t end_va;

    va &= 0xFFFFFF;
    end_va = va + count;

    // round down to sector boundary
    va &= ~(SECTSIZE - 1);

    // translate from bytes to sectors, and kernel starts at sector 1
    offset = (offset / SECTSIZE);

    // If this is too slow, we could read lots of sectors at a time.
    // We'd write more to memory than asked, but it doesn't matter --
    // we load in increasing order.
	readsect((uint8_t *) va, offset);
}



struct lheader *get_lheader(void *addr)
{
	uint8_t *p = (uint8_t *)addr;
	struct lheader *lh;
		
	for(uint32_t i = 0; i < 8096; i ++)
	{
		lh = (struct lheader *)p;
		if(lh->boot_flags == 0xAA55 && lh->linux_sig[0] == 'H'
		&& lh->linux_sig[1] == 'd' && lh->linux_sig[2] == 'r'
		&& lh->linux_sig[3] == 'S')
			return (struct lheader *)lh;
		p ++;
	}
	return 0;
}

void linux_move(void *addr, uint32_t size)
{
	char *kern_p;
	struct lheader *lh;
	uint32_t setup_size;

	kern_p = (char *) addr;

	lh = get_lheader(addr);

	lh->code32_start = LINUX_KERN_ADDR;
	lh->loadflags |= 0x80;
	lh->loadflags |= 0x10;
	lh->heap_end_ptr = 0x9800 - 0x200;
	lh->type_of_loader = 0x71;
	setup_size = (lh->setup_sectors + 1) * 512;
	kern_p += setup_size;

	memmove((void *)LINUX_REAL_ADDR, addr, setup_size);
	memmove((void *)LINUX_KERN_ADDR, (void *)kern_p, (size - setup_size));
}


