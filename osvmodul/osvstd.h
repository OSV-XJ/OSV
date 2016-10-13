#ifndef __KERNEL__

#include <stdint.h>

typedef unsigned long u64; 
typedef unsigned int u32; 
typedef unsigned short u16; 
typedef unsigned char u8; 

#endif /* end of the linux kernel define */



/*
* OSV VMM std vmmcall definition
* both for kernel and user space
*/

#define VMCALL_READ 0
#define VMCALL_OPEN 1
#define VMCALL_WRITE 2
#define VMCALL_CLOSE 3
#define VMCALL_VMCONS 4
#define VMCALL_STARTVM 5
#define VMCALL_SOCKET 6
#define VMCALL_APIC 7
#define VMCALL_SNULL_INIT 8
#define VMCALL_SNULL_TX 9
#define VMCALL_SNULL_GET 11
#define VMCALL_SNULL_INTERNAL 12
#define VMCALL_GETDID 13

static __inline int vmmread(void)
{
int addr;
int ret;

addr = 0;
__asm __volatile("vmmcall\n\t"
					  :"=a"(ret)
					  :"a"(VMCALL_READ), "D"(addr)
					  :"cc");
return ret;
}

static __inline int vmmopen(void)
{
return 0;
}

static __inline int vmmwrite(void)
{
return 0;
}

static __inline int vmmclose(void)
{
return 0;
}

static __inline int vmmcons(u64 addr)
{
u64 adr;
int ret;

adr = addr;
__asm __volatile("vmmcall\n\t"
					  :"=a"(ret)
					  :"a"(VMCALL_VMCONS), "D"(addr)
					  :"cc");
return ret;
}

static __inline int start_vm(int id)
{
int ret;

__asm __volatile("vmmcall\n\t"
					  :"=a"(ret)
					  :"a"(VMCALL_STARTVM), "D"(id)
					  :"cc");
return ret;
}

static __inline uint64_t vmm_snull_init(u64 addr, int id)
{
	uint64_t ret1, ret;
	__asm __volatile("vmmcall\n\t"
					  :"=a"(ret1),"=b"(ret)
					  :"a"(VMCALL_SNULL_INIT), "D"(addr), "S"(id)
					  :"cc");

	return ret;
}

static __inline u64 vmm_snull_get(int id)
{
	u64 ret;
	__asm __volatile("vmmcall\n\t"
						:"=D"(ret)
						:"a"(VMCALL_SNULL_GET), "S"(id)
						:"cc");
	return ret;
}

static __inline int vmm_snull_internal(void)
{
	int ret;
	__asm __volatile("vmmcall\n\t"
						:"=a"(ret)
						:"a"(VMCALL_SNULL_INTERNAL)
						:"cc");
	return ret;
}

static __inline int vmm_get_did(void)
{
	int ret;
	__asm __volatile("vmmcall\n\t"
						:"=D"(ret)
						:"a"(VMCALL_GETDID)
						:"cc");
	return ret;
}
