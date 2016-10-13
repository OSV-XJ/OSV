#include <inc/types.h>
#include <inc/guestos.h>

#define PAGE_SHIFT 12

#define PAGE_SIZE (1UL<<PAGE_SHIFT)
#define PAGE_OFF (PAGE_SIZE - 1)
#define PAGE_MASK (~PAGE_OFF)


#define PAGE_TABLE_OFFSET ((1UL<<10) - 1)
#define PAGE_L_SHIFT(x) (PAGE_SHIFT + 9*(x-1))


#define PAGE_L_MASK(x) (PAGE_TABLE_OFFSET<<PAGE_L_SHIFT(x))
#define PAGE_L_OFF(x, vaddr) ((PAGE_L_MASK(x)&vaddr)>>PAGE_L_SHIFT(x))


pml4e vmm_get_guest_pml4_phy(pgd pgd, uint64_t vaddr)
{
	return (pml4e)(((uint64_t) pgd[PAGE_L_OFF(4, vaddr)])&PAGE_MASK);
}

pdpe vmm_get_guest_pdpe_phy(pml4e pml4, uint64_t vaddr)
{
	return (pdpe)(((uint64_t) pml4[PAGE_L_OFF(3, vaddr)])&PAGE_MASK);
}

pde vmm_get_guest_pde_phy(pdpe pdpe, uint64_t vaddr)
{
	return (pde)(((uint64_t) pdpe[PAGE_L_OFF(2, vaddr)])&PAGE_MASK);
}

pte vmm_get_guest_pte_phy(pde pde, uint64_t vaddr)
{
	return (pte)(pde[PAGE_L_OFF(1, vaddr)]&PAGE_MASK);
}

uint64_t vmm_get_guest_virt_to_phy(uint64_t cr3, uint64_t vaddr)
{
	pml4e pml4 = vmm_get_guest_pml4_phy((pgd) cr3, vaddr);
	if(!pml4)
		return -1;
	pdpe pdp = vmm_get_guest_pdpe_phy(pml4, vaddr);
	if(!pdp)
		return -2;
	pde pd = vmm_get_guest_pde_phy(pdp, vaddr);
	if(!pd)
		return -3;
	pte pt = vmm_get_guest_pte_phy(pd, vaddr);
	if(!pt)
		return -4;
	
	return (uint64_t)((pt&PAGE_MASK)|(vaddr&PAGE_OFF));
}
