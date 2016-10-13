#include <inc/mp.h>
#include <inc/stdio.h>
#include <inc/string.h>

#define VMPT_ADDR  0x500
#define VFPT_ADDR  0x200

extern void *mp_table;

static void *dump_mpt(struct mp_fptr *fptr)
{
	struct mp_conf_header *hd =
		(struct mp_conf_header *)(uint64_t)fptr->tb_addr;
	uint8_t sum = 0;
	uint32_t addr;
	
	
	mp_table = tmalloc(hd->base_t_length);
	if(!mp_table)
		return NULL;
	memcpy(mp_table, hd, hd->base_t_length);
	memcpy((void *)VMPT_ADDR, hd, hd->base_t_length);
	memcpy((void *)VFPT_ADDR, fptr, fptr->lenth * 16);

	struct mp_fptr *tmp = (struct mp_fptr *) VFPT_ADDR;
	tmp->tb_addr = 0;
	addr = VMPT_ADDR;
	
	tmp->tb_addr = (addr & 0xfff);
	char *p = (char *)tmp;
	for(int i = 0; i < tmp->lenth * 16; i ++)
		sum += p[i];
	sum -= p[10];
	p[10] = ((1<<8) - sum) & 0xFF;
	
	//fptr->tb_addr = (uint32_t)((uint64_t)mp_table);
	//fptr->tb_addr = (uint32_t)((uint64_t)mp_table & 0xFFFFFFFF);
	return mp_table;
}

static void restart_mpt(void *addr, void *orig)
{
	struct mp_conf_header *des = (struct mp_conf_header *) orig;
	memcpy(addr, orig, des->base_t_length);
}

void *create_mpt(void *addr);

void *mpt_domain_set(int id, void *addr);



