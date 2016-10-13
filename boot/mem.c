/*
 * this mainly jumps into to real kernel
 * 
 */
#include <inc/boot.h>

#define PML1 ((unsigned long long *)0xa000) 
#define PML2 ((unsigned long long *)0xb000)
#define PML3 ((unsigned long long *)0xc000)
#define PML4 ((unsigned long long *)0xd000)

void setup_pagetable(void)
{
	unsigned int i;
	*PML1 = 0xb000;
	*PML2 = 0xc000;
	*PML3 = 0xd000;
	for(i = 0; i < 0x22; i ++)
	{
		*(PML4 + i)= i*1000+0x7;
	}
}

void _set_cr4(unsigned int val)
{
	__asm __volatile("movl %%cr4, %%eax"::(cr4));
	return;
}

void longmode_jmp(void)
{
	_set_cr4();
	_set_cr3();
	_set_efer();
	_set_cr0();
	__asm __volatile("ljmp $0x8, %0":"=m"(long_m):);
}
