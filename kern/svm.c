#include <inc/npt.h>
#include <inc/x86.h>
#include <kern/console.h>
#include <inc/apic.h>
#include <inc/cpu.h>
#include <inc/percpu.h>
#include <inc/domain.h>

#include <inc/privacy.h>
#include <inc/pci.h>
#include <inc/iommu.h>

/*
 * preparing the stack for the guest to run
 * vmrun 
 * exit from the guest and restore the host state
 * code is copied from xen and corey
 */

volatile int npt_init = 0;

uint64_t shadow_npt;

uint64_t pte[512 * 8] __attribute__((aligned (4096), section(".data")));

uint64_t first[0xA0000] __attribute__((aligned (4096), section(".data")));

struct vmcb vmcb_test[32] __attribute__((aligned (4096), section (".data")));
vm_frame vfrm[32] __attribute__((aligned (4096), section (".data")));
char host_save[4096*32] __attribute__((aligned (4096), section (".data")));
uint8_t io_permap[4096*3*8]__attribute__((aligned(4096), section(".data")));
uint32_t *code32_addr = (uint32_t *)0x400fa;

extern void cga_init(void);

void construct_dev_npt(uint64_t dev_addr, int id, uint64_t n_cr3, uint64_t offset);

void msr_inter(void *pm, uint32_t msr, uint32_t acc)
{
	uint32_t base = 0x0, index = 0x0, offset = 0x0;

	uint8_t *map = (uint8_t *) pm;
	for(uint32_t i =  0; i < 8192; i ++)
		map[i] = 0x0;

	if(msr >= 0xc0000000 && msr < 0xc0010000)
		base = 0x800;
	if(msr >= 0xc0001000)
		base = 0x1000;
	
	index = msr&0xffff;
	offset = (index * 2)%8;
	index = (index * 2)/8;
	map = map + base + index;
	*map |= acc<<offset;
}


void wrmsr_inter(void *frame, void *vmcb)
{
	vm_frame *vf;
	struct vmcb *vb;
	uint32_t eax, edx, ecx;

	vf = frame;
	vb = vmcb;
	edx = 0xc0000080;
	eax = vb->rax;
	edx = vf->gregs.rdx;
	eax |= 0x1000;
	vb->efer = (((uint64_t) edx) << 32) | ((uint64_t) eax);
	vb->rip += 2;
}

void rest_stack_run_svm(struct vmcb *vmcb, vm_frame *vfrm1)
{
	uint64_t guest_stack = (uint64_t )vfrm1;
	uint64_t vmcb_phya = (uint64_t)vmcb;
	
	__asm __volatile(
	"mov %0, %%rsp \n\t"
	"mov %1, %%rdi \n\t"
	"jmp svm_run"
	::"r"(guest_stack),"r"(vmcb_phya)
	);
}

int iommu_inited = 0;

uint64_t *get_npt_pte(uint64_t addr, uint64_t ncr3)
{
	uint64_t *tmp;
	uint64_t phy_addr;

	phy_addr = ncr3 + ((addr>>39)&0x1ff)*8;
	tmp = (uint64_t *)phy_addr;
	phy_addr = *tmp;
	if((phy_addr&1)==0){
		return 0;
	}
	phy_addr &= 0xffffffffff000UL;

	phy_addr += ((addr>>30)&0x1ff) * 8;
	tmp = (uint64_t *)phy_addr;
	phy_addr = *tmp;
	if((phy_addr&1)==0){
		return 0;
	}
	phy_addr &= 0xffffffffff000UL;

	phy_addr += ((addr>>21)&0x1ff) * 8;
	tmp = (uint64_t *)phy_addr;
	phy_addr = *tmp;
	if((phy_addr&1)==0){
		return 0;
	}
	phy_addr &= 0xffffffffff000UL;

	phy_addr += ((addr>>12)&0x1ff) * 8;
	tmp = (uint64_t *)phy_addr;
	return tmp;
}

void remap_npt_page(uint64_t virt_addr, uint64_t phy_addr, uint64_t flags, uint64_t ncr3)
{
	uint64_t *pte_entry;
	pte_entry = get_npt_pte(virt_addr, ncr3);
	if(!pte_entry){
		init_npt_range(virt_addr, 4096, 0, 0);
		pte_entry = get_npt_pte(virt_addr, ncr3);
	}
	*pte_entry = phy_addr & ~0xfffUL;
	*pte_entry |= flags;
}

void start_svm(int id)
{
	//	extern struct boot_params *boot_params;
	uint32_t procid;
	extern struct ivrs *ivrs;

	
	procid = lapicid();
	
	struct vmcb *vmcb = &vmcb_test[procid];

	char *p = (char *)vmcb;
	pda[procid].vmcb_addr = (uint64_t)vmcb;
	
	disable_mce();
	write_msr(0xc0010117, ((uint64_t) host_save) + procid*4096);
	
	for(uint32_t i = 0; i < sizeof(struct vmcb); i ++)
		p[i] = 0;
	
	svm_vmcb_init(vmcb, id);

	if(domains[id].dom_type != 1){
		vmcb->intercepts1 |= INTERCEPT_CPUID;
	}

	if(node[id].bootpid!= read_pda(cpupid))
	{
		uint64_t val = read_msr(0xc0010114);
		write_msr(0xc0010114, val | 0x2);
		vmcb->exc_intercepts |= 1<<30; 
		// When control flow transfers to guest mode, CPU is setted in 16bit,
		// and uses the segement address (0x40000, in trampoline code),
		// 0x15c is the 'hlt' instruction code address in trampoline.
		vmcb->rip = 0x15c;
		vmcb->intercepts1 |= INTERCEPT_MSR_PROT;
		vmcb->msrpm_base_pa = (uint64_t)msrpm + 8192 * procid;
		msr_inter((void *)vmcb->msrpm_base_pa, 0xc0000080, 0x2);	// protect the svm enable bit from being cleaned
		vmcb->intercepts1 |= INTERCEPT_IOIO_PROT;
	//	vmcb->intercepts1 |= INTERCEPT_INTR;
		vmcb->iopm_base_pa = (uint64_t)&io_permap[4096*3*id];
		while(!npt_init);
		enable_npt(vmcb, id);
	}else{
		// For bp, control flow transfers to trampoline at svm_jmp(0x47),
		// and the code32_addr pointer points to the svm_sig variant which
		// controls where VMM should transfers the control flow to guest OS.
		void *tmp = madt_start;
		uint64_t page_flags;
		uint64_t phy_page;

		if(domains[id].dom_type == 1){ //This is a Linux domain
			*code32_addr =(uint32_t) domains[id].boot_params->hdr.code32_start;
			if(id > 0)
			{
				domains[id].boot_params = (struct boot_params *)((uint64_t)domains[id].boot_params - node[id].base_addr);
	//			lock_cprintf("code32_addr:%x\n", *code32_addr);
	//			lock_cprintf("boot_params:%lx\n", (uint64_t)domains[id].boot_params);
			}
		}else{	// This is a Xen domain
			*code32_addr = domains[id].code32_start;
			vfrm[procid].gregs.rbx = domains[id].mb_info_addr;
		}

		if(id > 0)
		{
			port_intercept_set((void *)vmcb->iopm_base_pa, 0x20);
			port_intercept_set((void *)vmcb->iopm_base_pa, 0x21);
			port_intercept_set((void *)vmcb->iopm_base_pa, 0xA0);
			port_intercept_set((void *)vmcb->iopm_base_pa, 0xA1);
		}

		enable_npt(vmcb, id);
		vmcb->intercepts1 |= INTERCEPT_IOIO_PROT;
		//	vmcb->intercepts1 |= INTERCEPT_INTR;
		vmcb->iopm_base_pa = (uint64_t)&io_permap[4096*3*id];
		//	port_intercept_set((void *)vmcb->iopm_base_pa, 0xcfc);

		if(id == 0){
			// virtualize acpi madt
			phy_page = (uint64_t)madt_page;
			page_flags = 5 | 2<<9 | 3UL<<61;
			for(; tmp<madt_end; tmp+= 4096){
				remap_npt_page((uint64_t)tmp, phy_page, page_flags, vmcb->n_cr3);
				phy_page += 4096;
			}
		}

		npt_init = 1;
		node[id].booted = 1;

/*		if(!iommu_inited){
			iommu_init(ivrs);
		} */
		lock_cprintf("Starting Domain: %d, boot_params: %lx\n", id, (uint64_t)domains[id].boot_params);
	}

	vfrm[procid].gregs.rsi = (uint64_t)domains[id].boot_params;
	vfrm[procid].gregs.rax = (uint64_t)vmcb;
	rest_stack_run_svm(vmcb, &vfrm[procid]);
}

int vmmcall_intr_des(struct generl_regs *regs)
{
	uint32_t data;
	uint32_t ioapic_base_addr = ioapic[0]->mmioapic_addr;

	if(regs->rbx == 0){
		data = 0x1a03a;
		mul_write_ioapic(0x10 + 2 * 10,
				data, ioapic_base_addr);
		data = 0x20<<24;	// destinatioin
		mul_write_ioapic(0x11 + 2 * 10,
				data, ioapic_base_addr);
		data = 0xa03a;
		mul_write_ioapic(0x10 + 2 * 10,
				data, ioapic_base_addr);
	}else{
		data = 0x1a03a;
		mul_write_ioapic(0x10 + 2 * 10,
				data, ioapic_base_addr);
		data = 0x44<<24;	// destinatioin
		mul_write_ioapic(0x11 + 2 * 10,
				data, ioapic_base_addr);
		data = 0xa03a;
		mul_write_ioapic(0x10 + 2 * 10,
				data, ioapic_base_addr);

	}
	lock_cprintf("Hkey, %s\n", __func__);
	return 0;
}

void start_domain(int id)
{
	extern uint8_t lapicid_to_index[256];
	int i, j = read_pda(cpupid) + 1, index;
	for(i=1; i < node[id].index; i ++, j++){
		index = lapicid_to_index[j];
		cpus[index].svm = 1;
	}
	start_svm(id);
}

static void set_apbootip(struct vmcb *vmcb)
{
	int np_enable = vmcb->np_enable;
	uint16_t *vec;
	uint32_t cs;
	uint64_t tmp;

	if(!np_enable)
	{
		vec = (uint16_t *)0x467;
		cs = vec[1];
		lock_cprintf("In %s, nested page table is not enabled\n", __func__);
	}
	else
	{
		uint64_t *ncr3 =(uint64_t *) (vmcb->n_cr3 & 0xfffffffffffffUL);
		uint64_t *pdpe =(uint64_t *)( ncr3[0] & ~((0x1UL << 12) - 1) & 0xfffffffffffffUL);
		uint64_t *pde = (uint64_t *)(pdpe[0] & ~((0x1UL << 12) - 1) & 0xfffffffffffffUL);
		uint64_t *pmd  =(uint64_t *) (pde[0] & ~((0x1UL << 12) - 1) & 0xfffffffffffffUL);
		uint8_t *pte = (uint8_t *)(pmd[0] & ~((0x1UL << 12) - 1) & 0xfffffffffffffUL);
		vec = (uint16_t *)&pte[0x467];
		cs  = vec[1];
			lock_cprintf("CPU:%ld &pte[0x467]: 0x%lx in set_apbootip\n", read_pda(cpupid), (uint64_t)&pte[0x467]);
		//	lock_cprintf("ip addr is 0x%lx in set_apbootip of np_enable\n", cs<<4);
		//lock_cprintf("ip addr is 0x%lx in set_apbootip of np_disable\n", ((uint16_t*)0x467)[1] << 4);

	}
	vmcb->cs.sel = cs;
	vmcb->rip = 0x0;
	vmcb->cs.base = cs<<4;
	vmcb->efer = 0x1000;
}

void svm_efer_pro(struct vmcb *vmcb)
{
	uint64_t val = read_msr(0xc0010114);
	val &= ~0x2;
	write_msr(0xc0010114, val);
	set_apbootip(vmcb);
}

uint64_t gos_msr[32];

static void msr_save(int id)
{
	struct osv_pda *cpu = &pda[id];
	__asm__ __volatile__("mfence":::"memory");
	gos_msr[id] = read_msr(0xc0000101);
	__asm__ __volatile__("mfence":::"memory");

	__asm__ __volatile__("mfence":::"memory");
	write_msr(0xc0000101, (uint64_t)cpu);
	__asm__ __volatile__("mfence":::"memory");
}

static void msr_reload(int id)
{
	__asm__ __volatile__("mfence":::"memory");
	write_msr(0xc0000101, gos_msr[id]);
	__asm__ __volatile__("mfence":::"memory");
}

int addr_ismem(uint64_t addr)
{
	int i;
	int entries = boot_params->e820_entries;
	struct e820entry *entry = boot_params->e820_map;
	for(i=0; i<entries; i++){
		if(addr>=entry[i].addr && addr<entry[i].addr+entry[i].size){
			if(entry[i].type == 1){
				lock_cprintf("%s address range start:%lx, end:%lx\n", __func__,
						entry[i].addr, entry[i].addr+entry[i].size);
				return 1;
			}else{
				return 0;
			}
		}
	}
	return 0;
}


uint64_t npt_travel1(uint64_t addr, uint64_t ncr3)
{
	uint64_t *tmp;
	uint64_t phy_addr;

	phy_addr = ncr3 + ((addr>>39)&0x1ff)*8;
	tmp = (uint64_t *)phy_addr;
	phy_addr = *tmp;
	if((phy_addr&1)==0){
		lock_cprintf("Not present\n");
		return 0;
	}
	phy_addr &= 0xffffffffff000UL;

	phy_addr += ((addr>>30)&0x1ff) * 8;
	tmp = (uint64_t *)phy_addr;
	phy_addr = *tmp;
	if((phy_addr&1)==0){
		lock_cprintf("Not present\n");
		return 0;
	}
	phy_addr &= 0xffffffffff000UL;

	phy_addr += ((addr>>21)&0x1ff) * 8;
	tmp = (uint64_t *)phy_addr;
	phy_addr = *tmp;
	if((phy_addr&1)==0){
		lock_cprintf("Not present\n");
		return 0;
	}
	phy_addr &= 0xffffffffff000UL;

	phy_addr += ((addr>>12)&0x1ff) * 8;
	tmp = (uint64_t *)phy_addr;
	phy_addr = *tmp;
	phy_addr &= 0xffffffffff000UL;
	return phy_addr + (addr & 0xfff);
}

uint64_t npt_travel(uint64_t addr, uint64_t ncr3)
{
	uint64_t *tmp;
	uint64_t phy_addr;

	phy_addr = ncr3 + ((addr>>39)&0x1ff)*8;
	tmp = (uint64_t *)phy_addr;
	phy_addr = *tmp;
	lock_cprintf("2:%lx-- ", phy_addr);
	if((phy_addr&1)==0){
		lock_cprintf("Not present\n");
		return 0;
	}
	phy_addr &= 0xffffffffff000UL;

	phy_addr += ((addr>>30)&0x1ff) * 8;
	tmp = (uint64_t *)phy_addr;
	phy_addr = *tmp;
	lock_cprintf("3:%lx-- ", phy_addr);
	if((phy_addr&1)==0){
		lock_cprintf("Not present\n");
		return 0;
	}else{
		if((phy_addr & 0x80) != 0){
			lock_cprintf("1G page\n");
			phy_addr &= ~0x3fffffff;
			return phy_addr + (addr & 0x3fffffff);
		}
	}
	phy_addr &= 0xffffffffff000UL;

	phy_addr += ((addr>>21)&0x1ff) * 8;
	tmp = (uint64_t *)phy_addr;
	phy_addr = *tmp;
	lock_cprintf("4:%lx-- ", phy_addr);
	if((phy_addr&1)==0){
		lock_cprintf("Not present\n");
		return 0;
	}else{
		if((phy_addr & 0x80) != 0){
			lock_cprintf("2M page\n");
			phy_addr &= ~0x1fffff;
			return phy_addr + (addr & 0x1fffff);
		}
	}
	phy_addr &= 0xffffffffff000UL;

	phy_addr += ((addr>>12)&0x1ff) * 8;
	tmp = (uint64_t *)phy_addr;
	phy_addr = *tmp;
	lock_cprintf("pte:%lx, tmp:%p\n", phy_addr, tmp);
	phy_addr &= 0xffffffffff000UL;
	return phy_addr + (addr & 0xfff);
}

struct vmcb *privacy_vmcb;

void cpuid_intercept(struct vmcb *vmcb, struct generl_regs *gregs)
{
	uint32_t eax, ebx, ecx, edx;

	eax = (uint32_t) vmcb->rax;

	__asm__ __volatile__(
			"cpuid\n\t"
			:"=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
			:"a"(eax));
	if(vmcb->rax == 0x80000001){
		ecx &= ~(uint32_t) 0x4;
	}

	vmcb->rax = (uint64_t) eax;
	gregs->rbx = (uint64_t) ebx;
	gregs->rcx = (uint64_t) ecx;
	gregs->rdx = (uint64_t) edx;
	vmcb->rip += 2; 
	return;
}

//void svm_exit_handler()
void svm_exit_handler(uint64_t orig_rsp)
{
	struct vmcb *vmcb;
	uint32_t procid;
	int did;

	__asm __volatile("mov %%rax, %0":"=a"(vmcb)::"cc");

	procid = lapicid();

	msr_save(procid);
	if((uint64_t)vmcb != (uint64_t)&vmcb_test[procid]){
		lock_cprintf("VMCB not equal\n");
		vmcb = &vmcb_test[procid];
	}
	//	vmcb = (struct vmcb *)read_pda(vmcb_addr);
	//__asm __volatile("mov %%rax, %0":"=a"(vmcb)::"cc");

	privacy_vmcb = vmcb;
	if(vmcb->event_inj.v ==1){
		while(1)
			lock_cprintf("Event inject set\n");
	}

	//lock_cprintf("In svm_exit_handler, orig_rsp:%lx\n", orig_rsp);

	//	lock_cprintf("In svm_exit_handler, exitcode:%lx, rip:%lx",
	//			vmcb->exitcode, vmcb->rip);

	if(vmcb->exitinfo.v != 0){
		//handle_idt_interrupt_intercepts(vmcb);
		//		irq_inject(vmcb, vmcb->exitinfo.vector);
		vmcb->event_inj = vmcb->exitinfo;
		vmcb->event_inj.v = 1;
	}else{
		switch(vmcb->exitcode)
		{
			case 0x13:	//CR3 write
				//intercept_cr3_op(vmcb, &vfrm[procid].gregs);
				break;
			case 0x41:
				//step_handle(vmcb, &vfrm[procid].gregs);
				break;
			case 0x5e:
				svm_efer_pro(vmcb);
				break;
			case 0x72:
				cpuid_intercept(vmcb, &vfrm[procid].gregs);
				break;
			case 0x7b:
				io_intercept(vmcb);
				break;
			case 0x7c:
				//lock_cprintf("efer write intercept, msr addr is %x\n", vfrm[procid].gregs.rcx);
				wrmsr_inter(&vfrm[procid], vmcb);
				break;
			case 0x80:
				while(1)
					lock_cprintf("Guest executes vmm_run instruction, rip:%lx\n", vmcb->rip);
				break;
			case 0x81:
				privacy_vmcb = vmcb;
				vmcall(vmcb->rax, &vfrm[procid].gregs);
				vmcb->rip +=3;
				//		lock_cprintf("0x81 VMMCALL Handler, number:%ld\n", vmcb->rax);
				break;
			case 0x400:
				if(!addr_ismem(vmcb->exitinfo2)){
					if(vmcb->exitinfo2>=(uint64_t)madt_start && vmcb->exitinfo2<(uint64_t)madt_end){
						lock_cprintf("This is a madt write operation, exitinfo1:%lx\n", vmcb->exitinfo1);
						lock_cprintf("madt_start:%p, madt_end:%p, exitinfo2:%lx\n",
								madt_start, madt_end, vmcb->exitinfo2);
						lock_cprintf("Guest IP is :%lx\n", vmcb->rip);
						while(1);
					} 
					did = read_pda(cpudid);
					if(did != 0){
						lock_cprintf("Device NPT fault, %lx\n", vmcb->exitinfo2);
					}
					init_npt_range(vmcb->exitinfo2 & ~0xfff, 4096, did, 0);
				}else{
					lock_cprintf("NPT Fault occurred, address:%lx, rip:%lx\n",
							vmcb->exitinfo2, vmcb->rip);
					npt_travel(vmcb->exitinfo2, vmcb->n_cr3);
					npt_travel(vmcb->rip, vmcb->n_cr3);

					while(1);
				}
				break;
			default:
				lock_cprintf("Default exitcode:%lx, rip:%lx\n",
						vmcb->exitcode, vmcb->rip);
				while(1);
				break;
		}
	}
	msr_reload(procid);
}

void enable_npt(struct vmcb *vmcb, int did)
{
	extern char pgt_end[], pml4e[], npt[];
	uint64_t npt_size, npt_pml4e, npt_start;
	uint64_t *tmp;
	npt_size = (uint64_t) pgt_end - (uint64_t)pml4e;
	npt_pml4e = (uint64_t)&npt[did*npt_size];
	npt_start = node[did].base_addr + node[did].length - NPT_TAB_RESERVE_MEM;  //thrid level page table size

	if(!npt_init){
		tmp = (uint64_t *)npt_start;
		for(int i = 0; i < 160; i ++)
			//		tmp[i] =(uint64_t) &first[(0xA0000 * did)/8 + (0x1000 * i)/8] | 0x7; 
			tmp[i] =(uint64_t) &first[(0xA0000 * did)/8 + (0x1000 * i)/8] | 0x7 | (3UL<<61);
		memcpy(&first[(0xa0000*did)/8], (void*)0x0, 0xa0000);

	}

	vmcb->n_cr3 = npt_pml4e;
	vmcb->np_enable = 1;
}

void port_intercept_set(void *map, int port)
{
	int byte_index, bit_index;

	byte_index = port/8;
	bit_index = port%8;

	char *per = (char *)map;
	per[byte_index] |= 1 << bit_index;
}

void io_intercept(struct vmcb *vmcb)
{
	uint16_t port;
	int i;
	port = vmcb->exitinfo1 >> 16;
	vmcb->rip = vmcb->exitinfo2;
	switch(port){
		case 0x64:
			if((vmcb->exitinfo1 & 1) == 0){
				if(((vmcb->rax)&0xff)!=0xfe)
					return;
				//			debug_printf("Before domu reboot\n");
				lock_cprintf("Before dom reboot\n");
				//				reboot_domain();
			}
			break;
		case 0xcfc:			
			/*  I/O configuration space access, we should check the enable bit in port 0xcf8 */
			pci_config_access(vmcb);
			break;
	}
}

void irq_inject(struct vmcb *vmcb, uint64_t irq)
{
	vmcb->intr_t.vector = irq;
	vmcb->intr_t.tpr = 0xf;
	vmcb->intr_t.irq = 0x1;
	vmcb->intr_t.prio = 0xf;
	vmcb->intr_t.ign_tpr = 0x1;
}

void get_did(struct generl_regs *regs)
{
	int did;
	did = read_pda(cpudid);
	regs->rdi = did;
}

void get_dom_bid(struct generl_regs *regs)
{
	int bid;
	bid = node[regs->rbx].bootpid;
	regs->rbx = bid;
}
