#ifndef OSV_NPT
#define OSV_NPT

#include <inc/types.h>
#include <inc/svm.h>
#include <inc/stdio.h>

#define LAPIC_BASE 0xFEE00000
#define PAGE_SHIFT 12
#define PHY_LENGTH 48
#define PAGE_SIZE (1<<PAGE_SHIFT)
#define PAGE_MASK ~(PAGE_SIZE - 1)

#define PDPE_OFFSET(x) ((x >> 39) & 0x1FF)
#define PDE_OFFSET(x) ((x >> 30) & 0x1FF)
#define PMD_OFFSET(x) ((x >> 21) & 0x1FF)
#define PTE_OFFSET(x) ((x >> 12) & 0x1FF)

#define NPT_TAB_RESERVE_MEM (48UL*1024*1024*1024/512)	// each domain has 48G address space at most
#define SNULL_RESERVE_MEM (100*1024*1024)	// SNULL VNIC reservered 100M memory


/*naive pte get function for 4kB page size*/
static uint64_t *get_pte(uint64_t *cr3, uint64_t addr, int lvl)
{
	uint64_t *pdpe = &cr3[PDPE_OFFSET(addr)];
	if(!pdpe)
		return 0;
	if(lvl == 1)
		return pdpe;
	pdpe = (uint64_t *)(*pdpe & PAGE_MASK);
		
	uint64_t *pde = &pdpe[PDE_OFFSET(addr)];
	if(!pde)
		return 0;
	if(lvl == 2)
		return pde;
	pde = (uint64_t *)(*pde & PAGE_MASK);
	
	uint64_t *pmd = &pde[PMD_OFFSET(addr)];
	if(!pmd)
		return 0;
	if(lvl == 3)
		return pmd;
	pmd = (uint64_t *)(*pmd & PAGE_MASK);
		
	uint64_t *pte = &pmd[PTE_OFFSET(addr)];
	if(!pte)
		return 0;
	
	return pte;
}

int init_npt_apic(uint64_t *cr3);

void init_npt_range(uint64_t addr, uint64_t size, int did, uint64_t phy_offset);

#endif
