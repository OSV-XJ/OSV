#include <inc/types.h>

typedef struct mem_contrl_block
{
	size_t size;
	uint64_t last_addr;
	int aval;
	struct mem_contrl_block *prev, *next;
}mem_t;


void *tmalloc(size_t size);
int tfree(void *start);

void detect_mem();
void moveself_to_last_Gb(uint64_t addr);
