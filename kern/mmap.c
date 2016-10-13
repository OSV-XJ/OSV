#include <inc/stdio.h>
#include <inc/x86.h>
#include <inc/mmap.h>

extern char t_bss[], t_bss_end[];

mem_t *free;
int init = 0;

#define zone_addr(x) ((void *)(x + 1))

struct sem malloc_lock;


static int init_malloc()
{
//	lock_cprintf("free before init addr is 0x%x, ", free);
//	lock_cprintf("t_bss addr is 0x%x\n", t_bss);
	free = (mem_t *) t_bss;
	free->size = 4096 * 16 - sizeof(mem_t);
	free->last_addr = (uint64_t) t_bss_end;
	free->prev = free->next = NULL;
	free->aval = 1;
	malloc_lock.semph = 1;
//	lock_cprintf("after init addrs is 0x%x\n", free);
	init = 1;
	return 1;
}

/*simplest malloc and free, so naive*/

void *tmalloc(size_t size)
{
	mem_t *tmp, *new;

	if(!init)
		init_malloc();
	
	tmp = free;

/*	lock_cprintf("total bss size is %d, required size is %d\n",
					 tmp->size, size);*/
	spin_lock(&malloc_lock);
//	lock_cprintf("here, free addr %p, tmp addr %p\n", free, tmp);
	for(tmp = free; tmp != 0; tmp = tmp->next)
	{
		if(tmp->size >= size)
		{
			if(tmp->size > size + sizeof(mem_t))
			{
				new =(mem_t *)((uint64_t)(tmp + 1) + size);
				new->prev = tmp->prev;
				new->prev->next = new;
				new->next = tmp->next;
				new->last_addr = tmp->last_addr;
				new->size = tmp->size - size - sizeof(mem_t);
				new->aval = 1;
				tmp->size = size;
				tmp->last_addr = (uint64_t)new;
			}
			tmp->prev->next = tmp->next;
			tmp->next->prev = tmp->prev;
			tmp->aval = 0;
//			lock_cprintf("here in tmalloc, tmp addr is %p\n", tmp);
			spin_unlock(&malloc_lock);
			return zone_addr(tmp);
		}
	}
	spin_unlock(&malloc_lock);
	return NULL;
}

int tfree(void *start)
{
	mem_t *tmp, *cur;

	tmp = free;
	cur = (mem_t *)((uint64_t)start - sizeof(mem_t));

	spin_lock(&malloc_lock);
	
	while(tmp < cur)
	{
		if(!tmp->next)
			break;
		if(tmp->next > cur)
			break;
		tmp = tmp->next;
	}
	if(!tmp)
	{
		cur->next = free;
		cur->prev = NULL;
		free->prev = cur;
		free = cur;
		spin_unlock(&malloc_lock);
		return 0;
	}
		
	cur->prev = tmp;
	cur->next = tmp->next;
	tmp->next->prev = cur;
	tmp->next = cur;
	cur->aval = 1;
	
	/*Is it right?*/
	if(cur->last_addr == (uint64_t)cur->next)
	{
		cur->size = cur->next->size + cur->size + sizeof(mem_t);
		cur->last_addr = cur->next->last_addr;
		cur->next = cur->next->next;
		cur->next->next->prev = cur;
	}
	if(tmp->last_addr == (uint64_t)cur)
	{
		tmp->size = tmp->size + cur->size + sizeof(mem_t);
		tmp->last_addr = cur->last_addr;
		cur->next->prev = tmp;
		tmp->next = cur->next;
	}
	spin_lock(&malloc_lock);
	
	return 0;
}

void moveself_to_last_Gb(uint64_t addr)
{
	return;
}

void detect_me()
{
	return;
}
