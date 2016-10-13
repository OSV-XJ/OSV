#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/assert.h>
#include <inc/types.h>

#include <kern/monitor.h>
#include <kern/console.h>

struct e820_entry
{
	uint64_t start;
	uint64_t size;
	uint32_t	type;
};

#define E820_ENTRY ((struct e820_entry *) 0x3000)
#define E820_NR ((uint8_t *) 0x1e8)

void detect_mem(void)
{
	uint8_t i;
	struct e820_entry *entry;
	entry = E820_ENTRY;
	cprintf("total ranges %d\n", *E820_NR);
	for(i = 0; i < *E820_NR; i ++ )
	{
		cprintf("range %d\n", i);
		cprintf("start addr 0x%lx,	size 0x%lx,	type %d\n", entry->start, entry->size, entry->type);
		entry ++;
	}
}
