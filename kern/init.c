/* See COPYRIGHT for copyright information. */
#include <inc/boot1.h>
#include <inc/string.h>
#include <inc/assert.h>
#include <inc/mmap.h>
#include <inc/vmpt.h>
#include <inc/irq.h>
#include <inc/apic.h>
#include <inc/cpu.h>
#include <kern/monitor.h>
#include <kern/console.h>
#include <inc/svm.h>
#include <inc/domain.h>
#include <inc/percpu.h>
#include <inc/iommu.h>

// Test the stack backtrace function (lab 1 only)

struct sysx_info sys_io;
struct multiboot_header *mbh;
struct multiboot_info *mbinf;
memory_map_t *mem_map;
module_t *mdul;
struct boot_params *boot_params, st_params;
uint64_t d1_size;

char madt_page[4096*2] __attribute__((aligned (4096), section (".data")));
void *madt_start, *madt_end;

volatile uint32_t ap_svm = 0;

void reserve_modules(uint32_t mem_upper, int mod_count)
{
	uint32_t size = 0;
	uint64_t base_addr = 0; //last 32bit address
	int i = 0;

	mem_upper += 1024*1024;
	base_addr = mem_upper;
	
	for(; i<mod_count; i++){
		base_addr = (base_addr - 0x1000) & ~0xfff;
		size = mdul[i].mode_end - mdul[i].mode_start;
		base_addr = (base_addr - size) & ~0xfff;
		memcpy((void*)base_addr, (void*)(uint64_t)mdul[i].mode_start, size);
		mdul[i].mode_start = base_addr;
		mdul[i].mode_end = base_addr +size;
		if(mdul[i].string){
			strcpy((char*)base_addr+size, (char*)(uint64_t)mdul[i].string);
			mdul[i].string = base_addr+size;
		}

	}

#if 0

	base_addr = (base_addr - 0x1000) & ~0xfff;
	size = mdul[0].mode_end - mdul[0].mode_start;
	base_addr = (base_addr - size) & ~0xfff;
	memcpy((void*)base_addr, (void*)(uint64_t)mdul[0].mode_start, size);
	mdul[0].mode_start = base_addr;
	mdul[0].mode_end = base_addr +size;
	if(mdul[0].string){
		strcpy((char*)base_addr+size, (char*)(uint64_t)mdul[0].string);
		mdul[0].string = base_addr+size;
	}

	base_addr = (base_addr - 0x1000) & ~0xfff;
	size = mdul[1].mode_end - mdul[1].mode_start;
	base_addr = (base_addr - size) & ~0xfff;
	memcpy((void*)base_addr, (void*)(uint64_t)mdul[1].mode_start, size);
	mdul[1].mode_start = base_addr;
	mdul[1].mode_end = base_addr +size;
	if(mdul[1].string){
		strcpy((char*)base_addr+size, (char*)(uint64_t)mdul[1].string);
		mdul[1].string = base_addr+size;
	}

	base_addr = (base_addr - 0x1000) & ~0xfff;
	size = mdul[2].mode_end - mdul[2].mode_start;
	base_addr = (base_addr - size) & ~0xfff;
	memcpy((void*)base_addr, (void*)(uint64_t)mdul[2].mode_start, size);
	mdul[2].mode_start = base_addr;
	mdul[2].mode_end = base_addr +size;
	if(mdul[2].string){
		strcpy((char*)base_addr+size, (char*)(uint64_t)mdul[2].string);
		mdul[2].string = base_addr+size;
	}
#endif
}

void mboot_parse(struct multiboot_info *mf)
{
	mem_map = (memory_map_t *)((uint64_t) mf->mmap_addr);
	mdul = (module_t *)((uint64_t) mf->mods_addr);
	sys_io.cmdline = mf->cmdline;
	sys_io.e820_nents = 0;
//	lock_cprintf("mem_lower:%x, mem_upper:%x\n", mf->mem_lower, mf->mem_upper);
	reserve_modules(mf->mem_upper * 1024, mf->mods_count);
	for(;(uint64_t) mem_map < mbinf->mmap_addr + mbinf->mmap_length;
			mem_map = (memory_map_t *)((uint64_t)mem_map + mem_map->size + sizeof(mem_map->size)))
	{
		sys_io.e820_map[sys_io.e820_nents].addr = (uint64_t)mem_map->base_addr_low
			+ (((uint64_t) mem_map->base_addr_high)<<32); 
		sys_io.e820_map[sys_io.e820_nents].size = (uint64_t)mem_map->length_low +
			(((uint64_t) mem_map->length_high)<<32);
		sys_io.e820_map[sys_io.e820_nents].type = mem_map->type;
		sys_io.e820_nents ++;
	}
}

void hkey_dump_IRR()
{
	uint32_t queued = 0;
	int i;
	for(i=7; i>=0; i--){
		queued = read_lapic(0x200+i*0x10);
		if(queued){
			lock_cprintf("i:%d, queued is:%x\n", i, queued);
			queued = 0;
		}
	}
}

void ap_init()
{
	extern volatile int npt_init;
	*AP_BOOT_SIG = 0xdcba;
	load_idt();
	enable_lapic();
	{
		struct vmcb *vmcb;
		uint32_t procid = lapicid();
		vmcb = &vmcb_test[procid];
		lock_cprintf("the cpuid is %d, rip:%lx\n",
				procid, vmcb->rip);
	}

	cpus[lapicid()].booted = 1;
	cpu_init();
	if(read_pda(cpupid)>3)
		hkey_dump_IRR();
	while(!cpus[lapicid()].svm)
		__asm__ __volatile__("pause"::);
//	lock_cprintf("AP %d,%ld,%d started\n", lapicid(), read_pda(cpupid),read_lapic(0x20)>>24);
	if(read_pda(cpupid) == node[read_pda(cpudid)].bootpid){
		npt_init = 0;
		start_domain(read_pda(cpudid));
	}else{
		start_svm(read_pda(cpudid));
	}
}

struct ivrs *ivrs = 0;
struct madt *madt = 0;


void hkey_test(void)
{
#if 0
	volatile uint8_t *ioapic_addr[3] = {
		(volatile uint8_t *)0xfec00000,
		(volatile uint8_t *)0xfec80000,
		(volatile uint8_t *)0xfecc0000
	};
	int i = 0, j = 0, index = 0;
	uint32_t val, val_high;
	volatile uint32_t *tt;

	for(; i<3; i++){
		tt = (uint32_t *)(ioapic_addr[i] + 0x10);
		*ioapic_addr[i] = 0;
		val = *tt;
		val >>= 24;
		*ioapic_addr[i] = 1;
		index = *tt;

		lock_cprintf("tt:%p, IOAPIC ID: %d, version:%x, entries:%x, redirection table:\n",
				tt, val, index&0xff, index>>16);
		index >>= 16;
		index &= 0xff;
		for(j=0; j<index; j++){
			*ioapic_addr[i] = 0x10 + j*2;
			val = *tt;
			*ioapic_addr[i] = 0x10 + j*2 + 1;
			val_high = *tt;
			lock_cprintf("0x%x %x\n", val_high, val);

		}
	}
#endif
}

void hpet_dump(struct hpet* hpet)
{
	uint64_t base_adr;
	lock_cprintf("HPET Block id:%x, Address type:%x, bit witdh:%x offset:%x,\
			base_address:%lx, hpet_num:%x\n",
			hpet->block_id, hpet->space_type, hpet->bit_width, hpet->bit_offset,
			hpet->base_address, hpet->hpet_num);

}

	void
i386_init(uint32_t edi, uint32_t esi, uint32_t rdx, uint32_t ecx)
{
	extern char edata[], end[];
	uint32_t eax, ebx, epx, edx;


	// Before doing anything else, complete the ELF loading process.
	// Clear the uninitialized global data (BSS) section of our program.
	// This ensures that all static/global variables start out zero.
	memset(edata, 0, end - edata);

	cons_init();

	//	cprintf("edi 0x%x, esi 0x%x, rdx 0x%x, ecx 0x%x\n", edi, esi, rdx, ecx);
	memcpy(&st_params, (void *)((uint64_t) edi), sizeof(struct boot_params));
	boot_params = (struct boot_params *)((uint64_t)edi);
	boot_params->screen_info.orig_video_ega_bx = 0x10;


	if(rdx == 0x2badb002)
	{
		cprintf("multiboot ok\n");
		mbinf = (struct multiboot_info *)((uint64_t)  ecx);
		mboot_parse(mbinf);
	}
	else
	{
		struct sysx_info *tmp;
		tmp = (struct sysx_info *)((uint64_t) rdx);
		memcpy(&sys_io, tmp, sizeof(sys_io));
	}

	struct e820entry *entr;
	entr = sys_io.e820_map;

	struct rsdp *rsdp;
	rsdp = rsdp_get();

	struct srat *srat = 0;
	srat = srat_get((struct xsdt *) rsdp->xsdt_addr);

	struct hpet *hpet = 0;
	hpet = hpet_get((struct xsdt *) rsdp->xsdt_addr);

	ivrs = ivrs_get((struct xsdt *) rsdp->xsdt_addr);
	madt = madt_get((struct xsdt *) rsdp->xsdt_addr);
	madt_start = (void *)((uint64_t)madt & ~0xfffUL);
	madt_end = dump_madt_page(madt, madt_page);

	lock_cprintf("rsdp:%p, xsdt_addr:%lx, srat:%p, ivrs:%p, madt:%p\n",
			rsdp, rsdp->xsdt_addr, srat, ivrs, madt);

//	hpet_dump(hpet);

	if(rsdp)
	{
		cprintf("rsdp sig:");
		for(uint32_t i = 0; i < 8; i ++)
		{
			cprintf("%c", rsdp->sig[i]);
		}
		cprintf("\n");
	}

	init_8259A();
	set_interrupt();
	set_exception();
	load_idt();
	mp_init();

/*	cpuid(0x80000001, &eax, &ebx, &epx, &edx);
	if(epx & 0x4)
		lock_cprintf("SVM feature is contained in the CPU\n");

	if(read_msr(0xc0010114) & 0x10)
		lock_cprintf("SVM is disabled in BIOS\n");

	if(read_msr(0xc0000080) & 0x1000)
		lock_cprintf("SVM enabled \n");
	else
		lock_cprintf("SVM doesn't exists \n");
*/
	if(ivrs)
	{
		cprintf("ivrs sig:");
		for(uint32_t i = 0; i < 4; i ++)
		{
			cprintf("%c", ivrs->header.sig[i]);
		}
		cprintf("\n");
	}
	else 
		cprintf("ivrs no found \n");

	prepare_node_info(srat);

	/*
		for(uint32_t i = 0; i < 8; i ++)
		{
		cprintf("domain %d , cpuid is", i);
		for(uint32_t j = 0; j < node[i].index; j ++)
		{
		cprintf(" %d,", node[i].lapicid[j]);
		}
		cprintf(" mem start is 0x%lx, size is 0x%lx\n", node[i].base_addr, node[i].length);
		}
		*/
	percpu_init();
	cpu_init();

	smp_boot();

	//	create_maindomain(&st_params, &mdul[0]);
	create_domu(&st_params, &mdul[2], 1);
	create_domu(&st_params, &mdul[2], 2);
	create_domu(&st_params, &mdul[2], 3);
	create_domain(&st_params, &mdul[0], 0);

	create_mpt(gfptr);
	mpt_domain_set(0, (void *)VMPT_ADDR);
	//fake_acpi_lapic((struct madt*)(madt_page+ ((uint64_t)madt & 0xfffUL)), 0);

	start_domain(0);
	// Drop into the kernel monitor.
	monitor(NULL);
	while (1);
}


/*
 * Variable panicstr contains argument to first call to panic; used as flag
 * to indicate that the kernel has already called panic.
 */
static const char *panicstr;

/*
 * Panic is called on unresolvable fatal errors.
 * It prints "panic: mesg", and then enters the kernel monitor.
 */
	void
_panic(const char *file, int line, const char *fmt,...)
{
	va_list ap;

	if (panicstr)
		goto dead;
	panicstr = fmt;

	va_start(ap, fmt);
	cprintf("kernel panic at %s:%d: ", file, line);
	vcprintf(fmt, ap);
	cprintf("\n");
	va_end(ap);

dead:
	/* break into the kernel monitor */
	while (1)
		monitor(NULL);
}

/* like panic, but don't */
	void
_warn(const char *file, int line, const char *fmt,...)
{
	va_list ap;

	va_start(ap, fmt);
	cprintf("kernel warning at %s:%d: ", file, line);
	vcprintf(fmt, ap);
	cprintf("\n");
	va_end(ap);
}


