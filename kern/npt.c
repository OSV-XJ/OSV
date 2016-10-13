#include <inc/npt.h>
#include <inc/x86.h>
#include <inc/apic.h>
#include <kern/console.h>
#include <inc/cpu.h>
#include <inc/acpi.h>
#include <inc/domain.h>


uint64_t lapic_pte[512] __attribute__((aligned (4096), section(".data")));

void construct_npt_1G(uint64_t addr, uint64_t size, int did, uint64_t phy_offset);
void construct_npt_2M(uint64_t addr, uint64_t size, int did, uint64_t phy_offset);
void construct_npt_4K(uint64_t addr, uint64_t size, int did, uint64_t phy_offset);


int init_npt_apic(uint64_t *cr3)
{
	uint64_t *ncr3;
	uint64_t *lapic, *pmd;
	uint64_t val;
		
	ncr3 = (uint64_t *)cr3;
	pmd = get_pte(ncr3, LAPIC_BASE, 3);
	val = *pmd & PAGE_MASK;
	*pmd =(uint64_t)&lapic_pte[0] | 0x17;
	for(int i = 0; i < 512; i ++)
		lapic_pte[i] = val + i * 0x1000 + 0x17;

	lapic = get_pte(ncr3, LAPIC_BASE, 4);
	if(!lapic)
		return -1;
	val = *lapic & PAGE_MASK;
	*lapic = val | 0x15;
	lock_cprintf("lapic npt val is 0x%lx\n", *lapic);
	return 0;
}



static inline void flat_set_logicalid(uint32_t val)
{
	uint32_t logicalid;
	logicalid = val >> 24;
	cpus[lapicid()].logicalid = logicalid;
	/*
	 * mask all the logical id destination interupt including ipi and
	 * ioapic
	 */
	write_lapic(0x0, val);
}

static inline void ipi_send_logical(uint32_t val)
{
	uint32_t des, cpuid, domainid, msgtype;
	uint32_t pid, icrl;	
	des = read_lapic(0x310) >> 24;
	cpuid = lapicid();
	domainid = cpus[cpuid].nodeid;
	msgtype = ((val>>8) & 0x7);

	//	write_lapic(0x300,val);
	//	return;
	if(msgtype != 0)
	{
		write_lapic(0x300, val);
		return;
	}
		
	
	/* Destination mode: logical or physical */
	if(val & LOGI_DES)
	{
		switch((val >> 18)&0x3)
		{
		case 0x0:
			for(int i = 0; i < node[domainid].index; i ++)
				if(cpus[node[domainid].lapicid[i] - cpuid_offset].logicalid & des)
				{
					pid = node[domainid].lapicid[i] << 24;
					icrl = val;
					write_lapic(0x310, pid);
					icrl &= ~LOGI_DES;
					write_lapic(0x300, icrl);
				}
			break;
			/*self ipi OK!*/
		case 0x1:
			write_lapic(0x300, val);
			break;
		case 0x2:
			for(int i = 0; i < node[domainid].index; i ++)
			{
				pid = node[domainid].lapicid[i] << 24;
				icrl = val;
				write_lapic(0x310, pid);
				icrl &= ~LOGI_DES;
				icrl &= ~ALL;
				write_lapic(0x300, icrl);
			}
			break;
		case 0x3:
			for(int i = 0; i < node[domainid].index; i ++)
				if(node[domainid].lapicid[i] != cpuid)
				{
					pid = node[domainid].lapicid[i] << 24;
					icrl = val;
					write_lapic(0x310, pid);
					icrl &= ~LOGI_DES;
					icrl &= ~ALL;
					write_lapic(0x300, icrl);
				}			
			break;
		}
	}
	else
	{
		switch((val >> 18)&0x3)
		{
		case 0x0:
			write_lapic(0x300, val);
			break;
			/*self ipi OK!*/
		case 0x1:
			write_lapic(0x300, val);
			break;
		case 0x2:
			for(int i = 0; i < node[domainid].index; i ++)
			{
				pid = node[domainid].lapicid[i] << 24;
				icrl = val;
				write_lapic(0x310, pid);
				icrl &= ~LOGI_DES;
				icrl &= ~ALL;
				write_lapic(0x300, icrl);
			}
			break;
		case 0x3:
			for(int i = 0; i < node[domainid].index; i ++)
				if(node[domainid].lapicid[i] != cpuid)
				{
					pid = node[domainid].lapicid[i] << 24;
					icrl = val;
					write_lapic(0x310, pid);
					icrl &= ~LOGI_DES;
					icrl &= ~ALL;
					write_lapic(0x300, icrl);
				}			
			break;
		}
	}
}

int handle_apic_write(uint64_t ip, uint64_t addr)
{
	return 0;
}

int vmapic(struct generl_regs *regs)
{
	uint32_t apic_reg, val;
	apic_reg = regs->rdi;
	val = regs->rsi;
	switch(apic_reg)
	{
	case 0x20:
		break;
	case 0xD0:
		flat_set_logicalid(val);
		//write_lapic(apic_reg, val);
		break;
	case 0x300:
		//write_lapic(apic_reg, val);
		ipi_send_logical(val);
		break;
	default:
		write_lapic(apic_reg, val);
		break;
	}
	return 0;
}

uint64_t irq0_count = 0;

int irq0_forward(struct generl_regs *regs)
{
	int i = 1;
	extern struct vmcb *privacy_vmcb;
	for(;i < NR_DOMAINS; i++){
		if(node[i].booted){
			write_lapic(0x310, node[i].bootpid<<24);
			write_lapic(0x300, 0x30);
//lock_cprintf("Hkey irq0_forward");
		}
	}
//	lock_cprintf("Hkey irq0_forward, %ld, ip:%lx\n",
//			irq0_count++, privacy_vmcb->rip);
	return 0;
#if 0	
	if(node[1].booted)
	{
		write_lapic(0x310, node[1].bootpid<<24);
		write_lapic(0x300, 0x30);
	}
	return 0;
#endif
}
#if 0
int init_npt_range(uint64_t addr, uint64_t size, int did, uint64_t phy_offset)
{
	extern char pgt_end[], pml4e[], npt[];
	uint64_t npt_size, npt_start;
	uint64_t npt_pml4e, npt_pdpe, npt_pde;
	uint64_t *tmp;
	uint64_t m, n, p, q;

	npt_size = (uint64_t) pgt_end - (uint64_t)pml4e;
	npt_pml4e = (uint64_t)&npt[did*npt_size];
	npt_pdpe = npt_pml4e + 4096;
	npt_pde = npt_pdpe + 4096;
	tmp = (uint64_t *)npt_pml4e;
	*tmp = npt_pdpe + 7;
	*tmp |= 3<<9;
	*tmp |= 3UL<<61;
//	if(addr == 0x100000)
//		phy_offset = VMM_RESERVED - 0x100000;
	/* second level page table set */
	m = (addr >> 30) & 0x1ff;
	n = (size >> 30) & 0x1ff; 
	n += (size & 0x3fffffff)?1:0;
	n += m;
	tmp = (uint64_t *)npt_pdpe;
	tmp += m;
	for(; m<n; m++){
		*tmp = npt_pde + (m<<12)  + 7;
		*tmp |= 2<<9;
		*tmp |= 3UL<<61;
		tmp ++;
	}

	/* thrid level page table set */
	npt_start = node[did].base_addr + node[did].length - NPT_TAB_RESERVE_MEM; //thrid level page table size
	m = (addr >> 21) & 0x3ffff;
	n = ((size >> 21) & 0x3ffff);
	n += (size & 0x1fffff)?1:0;
	n += m;
	tmp = (uint64_t *)npt_pde;
	tmp += m;
	for(; m<n; m++){
		*tmp = npt_start + (m<<12) + 7;
		*tmp |= 1<<9;
		*tmp |= 3UL<<61;
		tmp ++;
	}

	/* last level page table set */
	m = (addr >> 12) & 0x7ffffff;
	n = ((size >> 12) & 0x7ffffff);
	n += m;
	npt_start = node[did].base_addr + node[did].length - NPT_TAB_RESERVE_MEM; 
	tmp = (uint64_t *)npt_start;
	tmp += m;
	if((size & 0xfff) != 0){
		n++;
	}
	for(; m<n; m++){
		*tmp = (m<<12) + 7 + phy_offset;
		*tmp |= 3UL<<61;
		tmp ++;
	}
	return 0;
}
#endif

void construct_npt_1G(uint64_t addr, uint64_t size, int did, uint64_t phy_offset)
{
	extern char pgt_end[], pml4e[], npt[];
	uint64_t npt_size;
	uint64_t npt_pml4e, npt_pdpe;
	uint64_t *tmp;
	uint64_t m, n;
	npt_size = (uint64_t) pgt_end - (uint64_t)pml4e;
	npt_pml4e = (uint64_t)&npt[did*npt_size];
	npt_pdpe = npt_pml4e + 4096;
	tmp = (uint64_t *)npt_pml4e;
	*tmp = npt_pdpe + 7;
	*tmp |= 3<<9;
	*tmp |= 3UL<<61;

	if((addr & 0x3fffffff) != 0 ||
			(phy_offset & 0x3fffffff) != 0){
		construct_npt_2M(addr, size, did, phy_offset);
		return;
		lock_cprintf("1G Start or PHY_OFFSET not align    ");
//		lock_cprintf("Start:%lx, phy_offset:%lx\n", addr, phy_offset);
	}
	lock_cprintf("Construct 1G page, start:%lx, size:%lx, offset:%lx\n",
			addr, size, phy_offset);


	/* second level page table set */
	m = (addr >> 30) & 0x1ff;
	n = (size >> 30) & 0x1ff; 
	n += m;
	tmp = (uint64_t *)npt_pdpe;
	tmp += m;
	for(; m<n; m++){
		*tmp = (m<<30)  + 7 + phy_offset;
		*tmp |= 3UL<<61 | 0<<9 | 1<<7;
		tmp ++;
	}
}
/* addr and phy_offset should all 2M align */
void construct_npt_2M(uint64_t addr, uint64_t size, int did, uint64_t phy_offset)
{
	extern char pgt_end[], pml4e[], npt[];
	uint64_t npt_size, npt_start;
	uint64_t npt_pml4e, npt_pdpe, npt_pde;
	uint64_t *tmp;
	uint64_t m, n;

	if((addr & 0x1fffff) != 0  ||
			(phy_offset & 0x1fffff) !=0 ){
		lock_cprintf("2M Start or PHY_OFFSET not align    ");
//		lock_cprintf("Start:%lx, phy_offset:%lx\n", addr, phy_offset);
		return;
	}

	lock_cprintf("Construct 2M page, start:%lx, size:%lx, offset:%lx\n",
			addr, size, phy_offset);

	npt_size = (uint64_t) pgt_end - (uint64_t)pml4e;
	npt_pml4e = (uint64_t)&npt[did*npt_size];
	npt_pdpe = npt_pml4e + 4096;
	npt_pde = npt_pdpe + 4096;
	tmp = (uint64_t *)npt_pml4e;
	*tmp = npt_pdpe + 7;
	*tmp |= 3<<9;
	*tmp |= 3UL<<61;
	/* second level page table set */
	m = (addr >> 30) & 0x1ff;
	n = (size >> 30) & 0x1ff; 
	n += (size & 0x3fffffff)?1:0;
	n += m;
	tmp = (uint64_t *)npt_pdpe;
	tmp += m;
	for(; m<n; m++){
		*tmp = npt_pde + (m<<12)  + 7;
		*tmp |= 2<<9 | 3UL<<61;
		tmp ++;
	}

	/* thrid level page table set */
	m = (addr >> 21) & 0x3ffff;
	n = ((size >> 21) & 0x3ffff);
	n += m;
	tmp = (uint64_t *)npt_pde;
	tmp += m;
	for(; m<n; m++){
		*tmp = (m<<21) + 7 + phy_offset;
		*tmp |= 0<<9 | 3UL<<61 | 1<<7;
		tmp ++;
	}
//	lock_cprintf("Construct 2M pages OK\n");
}

void construct_npt_4K(uint64_t addr, uint64_t size, int did, uint64_t phy_offset)
{
	extern char pgt_end[], pml4e[], npt[];
	uint64_t npt_size, npt_start;
	uint64_t npt_pml4e, npt_pdpe, npt_pde;
	uint64_t *tmp;
	uint64_t m, n;

//	lock_cprintf("Construct 4K page, start:%lx, size:%lx, offset:%lx\n",
//			addr, size, phy_offset);
	npt_size = (uint64_t) pgt_end - (uint64_t)pml4e;
	npt_pml4e = (uint64_t)&npt[did*npt_size];
	npt_pdpe = npt_pml4e + 4096;
	npt_pde = npt_pdpe + 4096;
	tmp = (uint64_t *)npt_pml4e;
	*tmp = npt_pdpe + 7;
	*tmp |= 3<<9;
	*tmp |= 3UL<<61;

	/* second level page table set */
	m = (addr >> 30) & 0x1ff;
	n = (size >> 30) & 0x1ff; 
	n += (size & 0x3fffffff)?1:0;
	n += m;
	tmp = (uint64_t *)npt_pdpe;
	tmp += m;
	for(; m<n; m++){
		*tmp = npt_pde + (m<<12)  + 7;
		*tmp |= 2<<9;
		*tmp |= 3UL<<61;
		tmp ++;
	}

	/* thrid level page table set */
	npt_start = node[did].base_addr + node[did].length - NPT_TAB_RESERVE_MEM; //thrid level page table size
	m = (addr >> 21) & 0x3ffff;
	n = ((size >> 21) & 0x3ffff);
	n += (size & 0x1fffff)?1:0;
	n += m;
	tmp = (uint64_t *)npt_pde;
	tmp += m;
	for(; m<n; m++){
		*tmp = npt_start + (m<<12) + 7;
		*tmp |= 1<<9;
		*tmp |= 3UL<<61;
		tmp ++;
	}

	/* last level page table set */
	m = (addr >> 12) & 0x7ffffff;
	n = ((size >> 12) & 0x7ffffff);
	n += m;
	tmp = (uint64_t *)npt_start;
	tmp += m;
	if((size & 0xfff) != 0){
		n++;
	}
	for(; m<n; m++){
		*tmp = (m<<12) + 7 + phy_offset;
		*tmp |= 3UL<<61;
		tmp ++;
	}
	return;
}

/*
 * Initial the domain nested page table, should ONLY be invoked
 * at the initial phase of guest domain 
 *
 */
void init_npt_range(uint64_t addr, uint64_t size, int did, uint64_t phy_offset)
{
	uint64_t m, n, p, q;


	if(size == 0){
		return;
	}

	if(size>=0x40000000UL && size - (addr & 0x3fffffff) >= 0x40000000UL){ // Memory range larger than 1G
		m = 0x40000000 - (addr & 0x3fffffff);	// Padding length
		m &= 0x3fffffff;
		n = addr + m;
		q = (size - m) & 0x3fffffff;	// Remaind memory length
		p = size - q - m;	// Continuous 1G page length
//		lock_cprintf("start:%lx, size:%lx, m:%lx, n:%lx, p:%lx, q:%lx\n", addr, size, m, n, p, q);

		init_npt_range(addr, m, did, phy_offset);	// init the padding page
		construct_npt_1G(n, p, did, phy_offset);
		init_npt_range(n+p, q, did, phy_offset);
		return;
	}

	if(size>=0x200000 && size - (addr & 0x1fffff) >= 0x200000){	// Memory range larger than 2M
		m = 0x200000 - (addr & 0x1fffff);
		m &= 0x1fffff;
		n = addr + m;
		q = (size - m) & 0x1fffff;
		p = size - q - m;
//		lock_cprintf("start:%lx, size:%lx, m:%lx, n:%lx, p:%lx, q:%lx\n", addr, size, m, n, p, q);
		init_npt_range(addr, m, did, phy_offset);
		construct_npt_2M(n, p, did, phy_offset);
		init_npt_range(n+p, q, did, phy_offset);
		return;
	}

	construct_npt_4K(addr, size, did, phy_offset);
	return;
}
