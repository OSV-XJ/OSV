#ifndef SVM_HEADER
#define SVM_HEADER

#include <inc/boot1.h>
#include <inc/types.h>
#include <inc/vmcb.h>
#include <inc/stdio.h>
#include <inc/acpi.h>

#define EFER_SVME 0x1000

struct generl_regs
{
	uint64_t rdi;
	uint64_t rsi;
	uint64_t rcx;
	uint64_t rdx;
	uint64_t r8;
	uint64_t r9;
	uint64_t r10;
	uint64_t r11;
	uint64_t rbp;
	uint64_t rbx;
	uint64_t r12;
	uint64_t r13;
	uint64_t r14;
	uint64_t r15;
	uint64_t rax;
}__attribute__ ((packed));

typedef union 
{
	uint64_t space[1024];
	struct generl_regs gregs;
}__attribute__ ((packed)) vm_frame;



static uint8_t iopm[4096*64] __attribute__((aligned (4096), section (".data")));
static uint8_t msrpm[4096*64] __attribute__((aligned (4096), section (".data")));

void svm_run(void);
void start_svm(int id);
void rest_stack_run_svm(struct vmcb *vmcb, vm_frame *vfrm1);
void svm_exit_handler(uint64_t);

//void svm_exit_handler(void);
void msr_inter(void *pm, uint32_t msr, uint32_t acc);
void svm_efer_pro(struct vmcb *vmcb);
void start_domain(int id);
void port_intercept_set(void *map, int port);
void io_intercept(struct vmcb *vmcb);
void irq_inject(struct vmcb *vmcb, uint64_t irqno);
void enable_npt(struct vmcb *vmcb, int did);
uint64_t shared_memory_map(uint64_t p_addr, uint64_t len, uint64_t n_cr3, int id);
extern void vmcall(uint64_t rax, struct generl_regs *gregs);
extern struct sysx_info sys_io;
extern void *madt_start, *madt_end;
extern char madt_page[4096*2];


static inline uint64_t virt_travel(uint64_t addr, uint64_t cr3, int level)
{
	uint64_t *tmp, value;
	uint32_t index;
	cr3 &= 0xffffffffff000UL;
	tmp = (uint64_t *)cr3;
	lock_cprintf("CR3:%lx\t", cr3);
	index = (addr >> 39) & 0x1ff;
	value = *(tmp + index);
	if((value & 1) == 0){
		lock_cprintf("Second page level non-present");
		goto out;
	}
	lock_cprintf("2:%lx, ", value);
	if(level == 2){
		lock_cprintf("\n");
		return value;
	}

	tmp = (uint64_t *)(value & 0xffffffffff000UL);
	index = (addr >> 30) & 0x1ff;
	value = *(tmp + index);
	if((value & 1) == 1){
		if((value&0x80)!=0){	//1G page
			lock_cprintf("3:%lx --- 1G page", value);
			goto out;
		}
	}else{
		lock_cprintf("Third page level non-present");
		goto out;
	}
	lock_cprintf("3:%lx, ", value);
	if(level == 3){
		lock_cprintf("\n");
		return value;
	}

	tmp = (uint64_t *)(value & 0xffffffffff000UL);
	index = (addr >> 21) & 0x1ff;
	value = *(tmp + index);
	if((value & 1) == 1){
		if((value&0x80)!=0){	//2M page
			lock_cprintf("4:%lx --- 2M page", value);
			goto out;
		}
	}else{
		lock_cprintf("Fourth page level non-present");
		goto out;
	}
	lock_cprintf("4:%lx, ", value);
	if(level == 4){
		lock_cprintf("\n");
		return value;
	}

	tmp = (uint64_t *)(value & 0xffffffffff000UL);
	index = (addr >> 12) & 0x1ff;
	value = *(tmp + index);
	if((value&1)==1){
		lock_cprintf("Addr:%lx", value);
	}else{
		lock_cprintf("Phy page not-present");
	}

out:
	lock_cprintf("\n");
	return -1UL;
}
static inline uint64_t phy_to_machine1(uint64_t addr, int did, uint64_t cr3, int type, int line)
{
	uint64_t pml4e, pdpe, pde, pte, pgt, ret_addr = 0;
	uint32_t *seg_des;
	uint32_t segment_base;


	pgt = (cr3 & 0xffffffffff000UL);
	if(did == 1){
		if(pgt>0x100000){
			pgt += node[1].base_addr - 0x100000;
		}else{
			lock_cprintf("Out\n");
			return 0;
		}
	}
	pml4e = *((uint64_t *)(pgt + ((addr>>39) & 0x1ff)*8));
//		lock_cprintf("\n pml4e: %lx\n", pml4e);
	pml4e = (pml4e & 0xffffffffff000UL);
	if(did == 1){
		if(pml4e>0x100000){
			pml4e += node[1].base_addr - 0x100000;
		}else{
			lock_cprintf("Out\n");
			return 0;
		}
	}
	pdpe = *((uint64_t *)(pml4e + ((addr>>30) &0x1ff)*8));
	if((pdpe&1)==1){
		if((pdpe & 0x80) != 0){	//1G page size
			pdpe = (pdpe  & 0xfffffc0000000UL);
			return pdpe+(addr&0x3fffffff);
		}
	}else{
		while(1)
		lock_cprintf("1G Null page, line:%d\n", line);
		return 0;
	}

	//		lock_cprintf("\n pdpe: %lx\n", pdpe);
	pdpe = (pdpe & 0xffffffffff000UL);
	if(did == 1){
		if(pdpe>0x100000){
			pdpe += node[1].base_addr - 0x100000;
		}else{
			lock_cprintf("Out\n");
			return 0;
		}
	}
	pde = *((uint64_t *)(pdpe + ((addr>>21) & 0x1ff)*8));
	if((pde&1)==1){
		if((pde & 0x80)!=0){ //2M page size
			pde = (pde & 0xfffffffe00000UL);
			return pde + (addr&0x1fffff);	
		}
	}else{
		while(1)
		lock_cprintf("2M Null page, line:%d\n", line);
		return 0;
	}

	//		lock_cprintf("\n pde: %lx\n", pde);
	pde = (pde & 0xffffffffff000UL);
	if(did == 1){
		if(pde>0x100000){
			pde += node[1].base_addr - 0x100000;
		}else{
			lock_cprintf("Out\n");
			return 0;
		}
	}
	pte = *((uint64_t *)(pde + ((addr>>12) & 0x1ff)*8));
			lock_cprintf("\n pte: %lx, %lx\n", pte, addr&0xfff);
	if((pte&1)==1){
		pte = (pte & 0xffffffffff000UL);
		return pte+(addr&0xfff);
	}else{
		while(1)
		lock_cprintf("4K Null page, line:%d\n", line);
		return 0;
	}
}

static inline uint64_t phy_to_machine(uint64_t addr, int did, struct vmcb *vmcb, int type, int line)
{
	uint64_t pml4e, pdpe, pde, pte, pgt, cr3, ret_addr = 0;
	uint32_t *seg_des;
	uint32_t segment_base;

	uint16_t seg_sel;

	if((vmcb->cr0 & 0x1) == 0){	// real mode
		if(type == 0){	// code segment
			ret_addr = vmcb->cs.sel & 0xffff;
			ret_addr = ret_addr << 4;
			ret_addr += addr;
			return ret_addr;
		}else{	//data segment, maybe other segment, who care!!!
			ret_addr = vmcb->ds.sel & 0xffff;
			ret_addr = ret_addr << 4;
			ret_addr += addr;
			return ret_addr;
		}
	}


	if((vmcb->efer & 0x100) ==0){	//protect mode, we should calculate the line address
		if((vmcb->cr0 & 0x80000000) == 0){	// No paging, must protect mode, we just use the legacy segment mode
			if(type == 0){	//code segment
				seg_sel = vmcb->cs.sel & 0xffff;
			}else{	//data segment
				seg_sel = vmcb->ds.sel & 0xffff;
			}
			if((seg_sel & 0x4) == 0){	//use GDT
				seg_des = (uint32_t *)(vmcb->gdtr.base + (seg_sel & 0xfff8));
			}else{	//use LDT
				seg_des = (uint32_t *)(vmcb->ldtr.base + (seg_sel & 0xfff8));
			}
			ret_addr = *seg_des >> 16;	//
			seg_des ++;
			ret_addr += (*seg_des & 0xff)<<16;
			ret_addr += (*seg_des & 0xff000000);
			ret_addr += addr;
			return ret_addr;
		}
		addr = ret_addr;
	}



	cr3 = vmcb->cr3;
	pgt = (cr3 & 0xffffffffff000UL);
	if(did == 2){
		if(pgt>0x100000){
			pgt += node[1].base_addr - 0x100000;
		}else{
			lock_cprintf("Out\n");
			return 0;
		}
	}
	pml4e = *((uint64_t *)(pgt + ((addr>>39) & 0x1ff)*8));
		lock_cprintf("\n pml4e: %lx\n", pml4e);
	pml4e = (pml4e & 0xffffffffff000UL);
	if(did == 2){
		if(pml4e>0x100000){
			pml4e += node[1].base_addr - 0x100000;
		}else{
			lock_cprintf("Out\n");
			return 0;
		}
	}
	pdpe = *((uint64_t *)(pml4e + ((addr>>30) &0x1ff)*8));
	if((pdpe&1)==1){
		if((pdpe & 0x80) != 0){	//1G page size
			pdpe = (pdpe  & 0xfffffc0000000UL);
			return pdpe+(addr&0x3fffffff);
		}
	}else{
		while(1)
		lock_cprintf("1G Null page, line:%d\n", line);
		return 0;
	}

			lock_cprintf("\n pdpe: %lx\n", pdpe);
	pdpe = (pdpe & 0xffffffffff000UL);
	if(did == 2){
		if(pdpe>0x100000){
			pdpe += node[1].base_addr - 0x100000;
		}else{
			lock_cprintf("Out\n");
			return 0;
		}
	}
	pde = *((uint64_t *)(pdpe + ((addr>>21) & 0x1ff)*8));
	if((pde&1)==1){
		if((pde & 0x80)!=0){ //2M page size
			pde = (pde & 0xfffffffe00000UL);
			return pde + (addr&0x1fffff);	
		}
	}else{
		while(1)
		lock_cprintf("2M Null page, line:%d\n", line);
		return 0;
	}

			lock_cprintf("\n pde: %lx\n", pde);
	pde = (pde & 0xffffffffff000UL);
	if(did == 2){
		if(pde>0x100000){
			pde += node[1].base_addr - 0x100000;
		}else{
			lock_cprintf("Out\n");
			return 0;
		}
	}
	pte = *((uint64_t *)(pde + ((addr>>12) & 0x1ff)*8));
		lock_cprintf("\n pte: %lx, %lx\n", pte, addr&0xfff);
	if((pte&1)==1){
		pte = (pte & 0xffffffffff000UL);
		return pte+(addr&0xfff);
	}else{
		lock_cprintf("4K Null page, line:%d, addr:%lx\n",
				line, addr);
		return 0;
	}
}


static inline void svm_vmload(void *vmcb)
{
	__asm__ __volatile__ (".byte 0x0f, 0x01, 0xda"
			: :"a"(vmcb):"memory");
}

static inline void svm_vmsave(void *vmcb)
{
	__asm__ __volatile__ (".byte 0x0f, 0x01, 0xdb"
			: :"a"(vmcb):"memory");
}

static void svm_vmcb_init(struct vmcb *vmcb, int id)
{
	/*Intercept cr0 write not read for notifying the hypervisor os'mode */
	vmcb->cr_intercepts = 0;//CR_INTERCEPT_CR0_WRITE;
	//	vmcb->cr_intercepts |= 0x80000; //CR3 write intercpet

	/*No DR intercepts*/
	vmcb->dr_intercepts = 0;

	/*No exception intercpet*/
	vmcb->exc_intercepts = 0;//0xffffffff;
	//	vmcb->exc_intercepts = 2;	//step debug

	//	vmcb->intercepts1 = 1;	// Intercept INTR

	/*general intercepts*/
	vmcb->intercepts2 = INTERCEPT_VMRUN
		| INTERCEPT_VMCALL
		| INTERCEPT_VMLOAD
		| INTERCEPT_VMSAVE
		| INTERCEPT_STGI
		| INTERCEPT_CLGI;

	vmcb->tsc_offset = 0;
	vmcb->cpl = 0;

	/**/
	vmcb->efer = EFER_SVME;
	vmcb->tlb_control = 0;
//	vmcb->tlb_control = 1;

	vmcb->guest_asid = id + 1;
	vmcb->np_enable = 0;
	vmcb->n_cr3 = 0;
	vmcb->lbr_virtual = 0;
	vmcb->intr_t.intr_masking = 0;

	vmcb->iopm_base_pa = (uint64_t)iopm;
	vmcb->msrpm_base_pa = (uint64_t)msrpm;

	vmcb->es.sel = 0x4000;
	vmcb->cs.sel = 0x4000;
	vmcb->ss.sel = 0x4000;
	vmcb->ds.sel = 0x4000;
	vmcb->fs.sel = 0x4000;
	vmcb->gs.sel = 0x4000;

	vmcb->es.attrib = 0x8a;
	vmcb->ss.attrib = 0x8a;
	vmcb->ds.attrib = 0x8a;
	vmcb->fs.attrib = 0x8a;
	vmcb->gs.attrib = 0x8a;
	vmcb->cs.attrib = 0x93;

	vmcb->es.limit = 0xffff;
	vmcb->cs.limit = 0xffff;
	vmcb->ds.limit = 0xffff;
	vmcb->ss.limit = 0xffff;
	vmcb->fs.limit = 0xffff;
	vmcb->gs.limit = 0xffff;

	vmcb->es.base = 0x40000;
	vmcb->ss.base = 0x40000;
	vmcb->cs.base = 0x40000;
	vmcb->ds.base = 0x40000;
	vmcb->fs.base = 0x40000;
	vmcb->gs.base = 0x40000;

	vmcb->idtr.base = 0;
	vmcb->idtr.limit = 256*4 - 1;

	vmcb->gdtr.base = 0;
	vmcb->gdtr.limit = 0xffff;

	vmcb->ldtr.base = 0;
	vmcb->ldtr.sel = 0;
	vmcb->ldtr.limit = 0xffff;
	vmcb->ldtr.attrib = 0x82;

	vmcb->tr.attrib = 0x08b;
	vmcb->tr.base = 0;
	vmcb->tr.limit = 0xffff;
	vmcb->tr.sel = 0;


	vmcb->rip = 0x47;
	vmcb->rsp = 0x9000;


	/*16 bit not in 32 bit*/
	vmcb->cr0 = 0x60000010;
	vmcb->cr2 = 0;
	vmcb->cr3 = 0;
	vmcb->cr4 = 0;
}

#endif
