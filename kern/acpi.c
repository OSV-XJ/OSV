#include <inc/acpi.h>
#include <inc/string.h>
#include <inc/stdio.h>
#include <inc/mmap.h>
#include <inc/cpu.h>

struct numa_node node[NR_NUMA];


static struct rsdp *rsdp_search(uint64_t base, uint32_t limit)
{
	uint32_t offset = 0;
	uint8_t *rsdp;

	while(offset < limit)
	{
		rsdp = (uint8_t *)(base + offset);
		/*if(rsdp->sig[0] == 'R' && rsdp->sig[1] == 'S' && rsdp->sig[2] == 'D' &&
		rsdp->sig[3] == ' ' && rsdp->sig[4] == 'P' && rsdp->sig[5] == 'T' &&
		rsdp->sig[6] == 'P' && rsdp->sig[7] == ' ')*/
		if(memcmp(rsdp, "RSD PTR ", 8) == 0)
			return (struct rsdp *)rsdp;
		offset += 16;
	}
	return 0;
}

struct rsdp *rsdp_get(void)
{
	uint8_t *data;
	uint16_t addr;
	struct rsdp *rsdp;
	data = (uint8_t *)400;
	addr = (data[0xe] | (data[0xf]<<8))<<4;
	if((rsdp = rsdp_search(addr, 1024)) != 0)
		return rsdp;
	if((rsdp = rsdp_search(0xe0000,0x1ffff)) != 0)
		return rsdp;
	return 0;
}

struct srat *srat_get(struct xsdt *xt)
{
	struct srat *aff;
	uint32_t index = 0;

	index = (xt->header.length - sizeof(struct des_header))/8;

	for(uint32_t i = 0; i < index; i ++)
	{
		aff = (struct srat *) xt->entry[i];
		if(memcmp(aff->header.sig, "SRAT", 4) == 0)
			return aff;
	}
	return 0;
}

struct ivrs *ivrs_get(struct xsdt *xt)
{
	struct ivrs *ivrs;
	uint32_t index = 0;

	index = (xt->header.length - sizeof(struct des_header))/8;

	for(uint32_t i = 0; i < index; i ++)
	{
		ivrs = (struct ivrs *) xt->entry[i];
		if(memcmp(ivrs->header.sig, "IVRS", 4) == 0)
			return ivrs;
	}
	return 0;
}

struct madt *madt_get(struct xsdt *xt)
{
	struct madt *madt;
	uint32_t index = 0;

	index = (xt->header.length - sizeof(struct des_header))/8;

	for(uint32_t i = 0; i < index; i ++)
	{
		madt = (struct madt*) xt->entry[i];
		if(memcmp(madt->header.sig, "APIC", 4) == 0)
			return madt;
	}
	return 0;
}

struct hpet *hpet_get(struct xsdt *xt)
{
	struct hpet *hpet;
	uint32_t index = 0;

	index = (xt->header.length - sizeof(struct des_header))/8;

	for(uint32_t i = 0; i < index; i ++)
	{
		hpet = (struct hpet*) xt->entry[i];
		if(memcmp(hpet->header.sig, "HPET", 4) == 0)
			return hpet;
	}
	return 0;
}

int prepare_node_info(struct srat *srat)
{
	char *p = 0;
	struct apic_aff *apic_aff = 0;
	struct mem_aff *mem_aff = 0;
	uint32_t domain_id;
	uint32_t entr_len;
	uint32_t i = 0, j = 0;
	entr_len = srat->header.length - sizeof(struct des_header) - 8;
	p = (char *)srat->entry;

	memset(node, 0, NR_NUMA*sizeof(struct numa_node));

	for(; p < (char *)srat->entry +  entr_len;)
	{
 		if(p[0] == 0 && p[1] == 16)
		{
			apic_aff = (struct apic_aff *)p;
			domain_id = apic_aff->proxi_domain;
			node[domain_id].id = domain_id;
			node[domain_id].lapicid[node[domain_id].index] = apic_aff->apic_id;
			if(node[domain_id].index == 0)
			{
				node[domain_id].bootpid = apic_aff->apic_id;
				cpus[lapicid_to_index[apic_aff->apic_id]].logicalid = 1;
			}
			node[domain_id].index ++;
			cpus[lapicid_to_index[apic_aff->apic_id]].nodeid= domain_id;
			p = p + 16;
			i ++;
		}
		else if(p[0] == 1 && p[1] == 40)
		{
			mem_aff = (struct mem_aff *)p;
			domain_id = (uint32_t) mem_aff->proxi_domain;
			node[domain_id].base_addr = (uint64_t) mem_aff->base_addr_low
				+ (((uint64_t) mem_aff->base_addr_high)<<32);
			node[domain_id].length = (uint64_t ) mem_aff->length_low
				+ (((uint64_t) mem_aff->length_high)<<32);
			lock_cprintf("node[%d] addr:%lx, size:%lx\n",
					domain_id, node[domain_id].base_addr, node[domain_id].length);

			p = p + 40;
			j ++;
		}else if(p[0]==2 && p[1]==24)
		{
			lock_cprintf("x2APIC entry\n");
			p += 24;
		}
		else{
			lock_cprintf("Bad srat entry, p[0]:%d, p[1]:%d\n", p[0], p[1]);
			return -1;
		}
	}
	return 0;
}


void *dump_madt_page(struct madt *madt, char * madt_page)
{
	void *madt_end;
	int pages;
	if(madt->header.length <= 4096){
		pages = madt->header.length + ((uint64_t)madt & 0xfff) > 0xfff ? 2 : 1;
	}else{
		pages = madt->header.length + ((uint64_t)madt & 0xfff) > 0xfff ? 2 : 1;
		pages += madt->header.length >> 12;
	}
	lock_cprintf("MADT length is %d page(s)\n", pages);

	madt_end = (void *)(((uint64_t)madt & ~0xfffUL) + pages * 4096);
	memcpy(madt_page, (void *)((uint64_t)madt & ~0xfffUL), pages * 4096);

	return madt_end;
}

int fake_acpi_lapic(struct madt *madt_fake, int nodeid)
{
	struct numa_node *nd = &node[nodeid];
	uint32_t len;
	struct lapic_entry *en;
	uint8_t sum = 0;
		
	/*4 bytes addr, 4 bytes flags*/
	len = madt_fake->header.length - sizeof(struct des_header) - 8;
	
	uint32_t index = 0;
	char *p = (char *)madt_fake->entry;
	char *t = (char *)madt->entry;
	lock_cprintf("madt length:%x, fake length:%x\n",
			madt->header.length, madt_fake->header.length);
	for(; p < (char *)madt_fake->entry + len; )
	{
		switch(p[0]){
			case 0:	// LAPIC
				if(t[4] == 1)
				{
					en = (struct lapic_entry *)p;
					if(!cpus[lapicid_to_index[en->lapicid]].nodeid == nodeid){
						en->flags = 0;
					}else{
						lock_cprintf("lapic id is %d, flags is %d\n", en->lapicid,
							en->flags);
						en->flags = 1;
					}

				}
				p += 8;
				t += 8;
				break;
			case 1:	// I/O APIC
				p += 12;
				t += 12;
				break;
			case 2:	// Interrupt Source Override
				p += 10;
				t += 10;
				break;
			case 3:	// NMI source
				p += 8;
				t += 8;
				break;
			case 4:	// Local APIC NMI
				p += 6;
				t += 6;
				break;
			case 5:	// Local APIC Address Override
				p += 12;
				t += 12;
				break;
			case 6:	// I/O SAPIC
				p += 16;
				t += 16;
				break;
			case 7:	// Local SAPIC
				p += p[1];
				t += p[1];
				break;
			case 8:	// Platform Interrupt Sources
				p += 16;
				t += 16;
				break;
			case 9:	// Processor x2APIC NMI
				p += 16;
				t += 16;
				break;
			case 0xa:	// Local x2APIC NMI
				p += 12;
				t += 12;
				break;
			case 0xb:	// GIC
				p += 40;
				t += 40;
				break;
			case 0xc:	// GICD
				p += 24;
				t += 24;
				break;
			default:
				lock_cprintf("APIC strcut type reserved\n");
				while(1);

		}
	}
	lock_cprintf("madt length:%x, fake length:%x\n",
			madt->header.length, madt_fake->header.length);

	p = (char *)madt_fake;
	for(int i = 0; i < madt_fake->header.length; i ++)
		sum += p[i];
	sum -= p[9];
	p[9] = (1<<8) - sum;
	lock_cprintf("madt length:%x, fake length:%x\n",
			madt->header.length, madt_fake->header.length);

	return 0;
}
