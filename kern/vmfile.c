#include <inc/types.h>
#include <inc/vmfile.h>
#include <inc/stdio.h>

long vm_read(struct generl_regs *regs)
{
	lock_cprintf("This is vmm read call \n");
	return 0;
}

long vm_write(struct generl_regs *regs)
{
	return 0;
}

long vm_open(struct generl_regs *regs)
{
	return 0;
}

long vm_close(struct generl_regs *regs)
{
	return 0;
}
