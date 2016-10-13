#include <inc/types.h>


extern uint64_t pml1[], pml2[], pml3[], pml4[], pml_tmp2[], pml_tmp3[], pml_tmp4[];

static uint32_t  pml_index(uint16_t level, uint64_t addr)
{
	return (uint32_t)((addr>>(9*level+12))&0x1ff);
}

static uint64_t page_index(uint64_t addr)
{
	return addr/0x1000;
}

static int set_pml_entry(uint64_t *pg_dir, uint32_t offset, uint64_t addr)
{
	*(pg_dir + offset) = addr;	
	return 1;
}
