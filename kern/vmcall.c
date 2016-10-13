#include <inc/types.h>
#include <inc/x86.h>

#define __NR_vmcall_max 32

#define __VMCALL(nr, sym) extern void sym(void);

#include <inc/osvstd.h>

#undef __VMCALL

#define __VMCALL(nr, sym) [nr] = sym,

extern void vm_ni_vmcall(void);



typedef void (*vm_call_ptr_t)(void);

const vm_call_ptr_t vm_call_table[__NR_vmcall_max + 1] = 
{
	[0 ... __NR_vmcall_max] = &vm_ni_vmcall,
	
	#include <inc/osvstd.h>
};


