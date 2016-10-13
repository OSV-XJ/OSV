#include <inc/types.h>
#include <inc/stdio.h>




typedef uint64_t pte;
typedef pte* pde;
typedef pde* pdpe;
typedef  pdpe* pml4e;
typedef  pml4e* pgd;

pml4e vmm_get_guest_pml4_phy(pgd pgd, uint64_t vaddr);

pdpe  vmm_get_guest_pdpe_phy(pml4e pml4e, uint64_t vaddr);

pde vmm_get_guest_pde_phy(pdpe pdpe, uint64_t vaddr);

pte vmm_get_guest_pte_phy(pde pde, uint64_t vaddr);

uint64_t vmm_get_guest_virt_to_phy(uint64_t cr3, uint64_t vaddr);
