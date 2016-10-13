//#include <inc/sem.h>
#ifndef PRIVACY_HEADER
#define PRIVACY_HEADER
#include <inc/x86.h>
#include <inc/svm.h>
#include <inc/guest_syscall.h>

void inline set_pte_flag(uint64_t *pte, uint64_t flag)
{
	*pte |= flag;
}

void inline clear_pte_flag(uint64_t *pte, uint64_t flag)
{
	*pte &= flag;
}

void intercept_cr3_op(struct vmcb *vmcb, struct generl_regs *regs);

void npt_fault_handle(struct vmcb *vmcb, struct generl_regs *regs);

void step_handle(struct vmcb *vmcb, struct generl_regs *regs);

void unmask_usr_mem_range(struct vmcb* vmcb, uint64_t addr_start, uint64_t addr_end);


uint64_t npt_travel(uint64_t addr, uint64_t cr3 );
uint64_t npt_travel1(uint64_t addr, uint64_t cr3 );
extern struct guest_brk_ops brk_record[32];
extern struct guest_unmap_ops unmap_record[32];


int start_protect(struct vmcb *vmcb);

int free_pages(struct vmcb *vmcb);


void set_privacy_npt(struct vmcb *vmcb);

void handle_idt_interrupt_intercepts(struct vmcb *vmcb);

extern struct vmcb *privacy_vmcb;

#define USR_NPT (600*1024*1024)
/* 一下宏定义需要根据内核实际情况调整 */
#define KERNEL_GS_OFFSET 0xb860
#define PER_CPU_SIZE 0x12b40
#define IDT_TABLE	0xffffffff819a5000UL
#define NMI_IDT_TABLE 0xffffffff819a6000UL
#define SYSTEM_CALL 0xffffffff81419e50UL
#define PAGE_FAULT 0xffffffff81419670UL
#define GENERAL_PROTECTION 0xffffffff81419640UL
#define DEV_NOT_AVAILABLE 0xffffffff8141b040UL
#define IRQ_ENTRIES_START 0xffffffff8141a540UL
#define IRQ_ENTRIES_END 0xffffffff8141a940UL
#define APIC_TIMER 0xffffffff8141aa40UL
#define INVALID_OP 0xffffffff8141b020UL
#define NMI_INTR 0xffffffff81419930UL

struct pages_free{
	uint32_t start;
	struct sem get_lock;
	uint32_t end;
	struct sem put_lock;
}* pfd;

struct mem_range{
	uint64_t start;
	uint64_t end;
};

struct addr_l{
	uint64_t start;
	uint64_t end;
	char *name;
	int prot;
	int sub_num;
};

struct sub_seg_l{
	uint64_t start;
	uint64_t end;
	int prot;
	int next;
};
#endif
