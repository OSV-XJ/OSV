#include <inc/vmpt.h>
#include <inc/cpu.h>
#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/pmap.h>
#include <inc/apic.h>

#define BOOTP 0x2
#define KERN_BASE 0x0


struct mp_fptr *gfptr;

uint32_t index = 0;
volatile struct cpu cpus[MAX_CPU];
uint8_t lapicid_to_index[256];
uint32_t ioapicid;
uint64_t cpu_id = 0;
uint64_t cpuid_offset = 0;
uint64_t cpunum = 0;
struct mp_ioapic *ioapic[16] = {
	NULL
};	// we support 8 ioapic at most


struct mp_fptr* mp_get_fptr(void)
{
	uint8_t *addr;
	uint16_t ebda;
	uint16_t base_boundary;
	struct mp_fptr *fptr;
	addr = (uint8_t *)0x400;
	ebda = (addr[0xf]<<8 | addr[0xe])<<4;
	base_boundary = (addr[0x14]<<16 | addr[0x13])*1024;
	/* 1st KB in the EBDA area */
	if((fptr = mp_search(0, 1024)))
		return fptr;
	/* last KB in the base memory */
	if((fptr = mp_search(639*1024, 1024)))
		return fptr;
	/* last KB in the BIOS memeory */
	if((fptr = mp_search(0xf0000, 0x10000)))
		return fptr;
	return 0;
}


int mp_init()
{
	struct mp_fptr *fptr;
	struct mp_conf_header *mp_header;
	struct mp_processer *proc;
	struct mp_busentry *bus;
	struct mp_iointr_assign *iointr = NULL;
	struct mp_locintr_assign *lintr;
	struct mp_ioapic *ioapic_entry;

	int ioapic_num = 0, mp_iointr_assign_num = 0;

	//apic_mtrr_init();

//	cprintf("lapic id is 0x%x\n", read_lapic(0x20));
	fptr = mp_get_fptr();
	if(!fptr)
		return -1;
	if(fptr->mp_feature[0]){
		cprintf("Other default configuration is implemented, %d\n", fptr->mp_feature[0]);
		getchar();
	}
	gfptr = fptr;
	lock_cprintf("fptr feature 1:%x, 2:%x\n", fptr->mp_feature[0], fptr->mp_feature[1]);

//	cprintf("mp table addr 0x%lx\n", fptr->tb_addr);
	mp_header = (struct mp_conf_header *)((uint64_t)  fptr->tb_addr + KERN_BASE);
	uint8_t *p = (uint8_t *)(mp_header + 1);
	int index = 0;

	for(; p < ((uint8_t *)mp_header + mp_header->base_t_length);)
	{
		switch(*p)
		{
			case PROC:
				proc = (struct mp_processer *) p;
				if(proc->lapicid == 0 && proc->lapicver == 0)
				{
					p += 20;
					continue;
				}
				if(proc->cpuflags & BOOTP)
				{
					cpus[index].bootp = 1;
				}
				cprintf("lapic version 0x%x and lapic id is 0x%x\n",
						proc->lapicver, proc->lapicid);
				cpus[index].lapicid = proc->lapicid;
				cpus[index].cpuid = proc->lapicid;
				cpus[index].nodeid = proc->lapicid;
				lapicid_to_index[proc->lapicid] = index;
				index++;
				p += 20;
				continue;
			case BUS:
				bus = (struct mp_busentry *) p;
				lock_cprintf("Bus %d is %c%c%c%c%c%c\n", bus->busid,
						bus->busstr[0], bus->busstr[1], bus->busstr[2],
						bus->busstr[3], bus->busstr[4], bus->busstr[5]);
				p += 8;
				continue;
			case IOAPIC:
				ioapic_entry = (struct mp_ioapic *)p;
				ioapic[ioapic_entry->ioapicid] = (struct mp_ioapic *)p;
				lock_cprintf("MP_TABLE I/O APIC id:%d\n", ioapic_entry->ioapicid);
		//		ioapic[ioapic_num] = (struct mp_ioapic *)p;
				ioapic_num ++;
				p += 8;
				continue;
			case IOINTR:
				iointr = (struct mp_iointr_assign *)p;
				mp_iointr_assign_num++;
				p += 8;
				continue;
			case LINTR:
				lintr = (struct mp_locintr_assign *)p;
				p += 8;
				continue;
			default:
				cprintf("error! Unkown mp conf type\n");
				return -1;
		}
	}
//	ioapic_init(ioapic, fptr);

	/*	ioapic_table(0xfec00000);
		getchar();
		ioapic_table(0xfec20000);
		getchar();*/

//		dump_ioapic(fptr);
//		getchar();
//	uint32_t max_entry = read_ioapic(0x1);
	enable_lapic();
//	lock_cprintf("the cpuid offset is %ld", cpuid_offset);

	cpunum = index;

	return 0;
}


int smp_boot(void)
{
	for(uint32_t i = 0; i < cpunum; i ++)
	{
		if(!cpus[i].bootp)
		{
			cpu_id = i * 4096;
//			lock_cprintf("Booting %d cpu\n", i);
			boot_ap(0x40000, cpus[i].cpuid);
//			lock_cprintf("Cpu %d is booted\n", i);
			while(!cpus[i].booted)
				__asm__ __volatile__("pause"::);
		}
	}

	return 0;
}
