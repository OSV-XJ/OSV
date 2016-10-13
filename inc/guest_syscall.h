#ifndef GUEST_SYSCALL_HEADER
#define GUEST_SYSCALL_HEADER
#include <inc/string.h>
#include <inc/svm.h>

void parse_guest_syscall(struct vmcb *vmcb, struct generl_regs *regs, int flag);
void register_trampline(uint64_t addr, uint64_t cr3);

uint64_t get_trampline_addr(uint64_t cr3);



struct guest_brk_ops{
	uint64_t flag;
	uint64_t addr;
};

struct guest_unmap_ops{
	uint64_t flag;
	uint64_t addr;
	uint64_t size;
};


#endif
