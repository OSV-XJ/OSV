#include <inc/vmpt.h>
#include <inc/acpi.h>

void *mp_table;

void *create_mpt(void *addr)
{
	dump_mpt((struct mp_fptr *)addr);
	return NULL;
}

int redump_mp()
{
	struct mp_fptr *fptr;
	struct mp_conf_header *mp_header;
	struct mp_processer *proc;
	struct mp_busentry *bus;
	struct mp_ioapic *ioapic;
	struct mp_iointr_assign *iointr;
	struct mp_locintr_assign *lintr;

	//apic_mtrr_init();
	//ioapic_tmp_map(0xfec00000);
	//ioapic_tmp_map(0xfec01000);

	//lapic_tmp_map(0xfee00000);
	//cprintf("lapic id is 0x%x\n", read_lapic(0x20));
	fptr = mp_get_fptr();
	if(!fptr)
		return -1;
	if(!fptr->mp_feature[0])
		cprintf("mp table exists\n");
	
	mp_header = (struct mp_conf_header *)((uint64_t ) fptr->tb_addr);
	return 0;
}

void *mpt_domain_set(int id, void *addr)
{
	struct numa_node *domain = &node[id];
	struct mp_conf_header *hd = (struct mp_conf_header *)addr;

	uint8_t *p = (uint8_t *)(hd + 1);
	uint8_t true = 0;
	uint8_t dom_first_mp_cpu_id;
	uint8_t flag = 0;
	
	for(; p < ((uint8_t *) hd +  hd->base_t_length);)
	{
		true = 0;
		if(*p == 0)
		{
			struct mp_processer *proc =(struct mp_processer *)p;
			for(int i = 0; i < domain->index; i ++)
				if(proc->lapicid == domain->lapicid[i])
				{
					true = 1;
					proc->cpuflags |= 1;
					lock_cprintf("Set CPU %d\n", proc->lapicid);
/*					if(proc->lapicid == domain->bootpid)
					{
						lock_cprintf("Domain %d bootpid:%d\n", id, proc->lapicid);
						proc->cpuflags |= 0x2;
					}else{
						proc->cpuflags &= ~0x2;
					}*/
					if(flag == 0){
						if(proc->lapicid != domain->bootpid){
							dom_first_mp_cpu_id = proc->lapicid;
							flag = 2;
							proc->lapicid = domain->bootpid;
						}else
							flag = 1;
						proc->cpuflags |= 0x2;
						break;
					}

					if(flag == 2){
						proc->lapicid = dom_first_mp_cpu_id;
						flag = 3;
			//			proc->cpuflags &= ~0x2;
			//			break;
					}

					proc->cpuflags &= ~0x2;
					break;
				}
			if(!true)
			{
				proc->cpuflags = 0;
			}
			p += 20;
		}
		else
			break;
	}
	uint8_t sum = 0;
	for(int i = 0; i < hd->base_t_length; i ++)
		sum += ((uint8_t *)hd)[i];
	sum -=  ((uint8_t *)hd)[7];
	((uint8_t *)hd)[7] = (1<<8) - sum;
	//	redump_mp();
	return hd;
}


