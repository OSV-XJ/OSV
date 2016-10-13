#include <inc/percpu.h>
#include <inc/x86.h>
#include <inc/apic.h>
#include <inc/cpu.h>


struct osv_pda pda[32];

DEFINE_PER_CPU(int, test);


int percpu_init(void)
{
	for(int i = 0; i < MAX_CPU; i ++ )
	{
//		pda[i].cpulid = cpus[i].lapicid - cpuid_offset;
		pda[i].cpulid = i;
		pda[i].cpupid = cpus[i].lapicid;
		pda[i].cpudid = cpus[i].nodeid;
	}
	return 0;
}

int cpu_init(void)
{
	int id = lapicid();
//	extern char percpu_start[], percpu_end[];
//	lock_cprintf("0x%lx, 0x%lx\n", percpu_end, percpu_start);
	struct osv_pda *cpu = &pda[id];

	int val = 0;

	__asm__ __volatile__("mfence":::"memory");
	write_msr(0xc0000101, (uint64_t)cpu);
	__asm__ __volatile__("mfence":::"memory");

	return 0;
}
