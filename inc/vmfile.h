#include <inc/types.h>
#include <inc/svm.h>


long vm_read(struct generl_regs *regs);
long vm_write(struct generl_regs *regs);
long vm_open(struct generl_regs *regs);
long vm_close(struct generl_regs *regs);


