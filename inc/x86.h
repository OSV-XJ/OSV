#ifndef JOS_INC_X86_H
#define JOS_INC_X86_H

#include <inc/types.h>
#include <inc/sem.h>

static __inline void breakpoint(void) __attribute__((always_inline));
static __inline uint8_t inb(int port) __attribute__((always_inline));
static __inline void insb(int port, void *addr, int cnt) __attribute__((always_inline));
static __inline uint16_t inw(int port) __attribute__((always_inline));
static __inline void insw(int port, void *addr, int cnt) __attribute__((always_inline));
static __inline uint32_t inl(int port) __attribute__((always_inline));
static __inline void insl(int port, void *addr, int cnt) __attribute__((always_inline));
static __inline void outb(int port, uint8_t data) __attribute__((always_inline));
static __inline void outsb(int port, const void *addr, int cnt) __attribute__((always_inline));
static __inline void outw(int port, uint16_t data) __attribute__((always_inline));
static __inline void outsw(int port, const void *addr, int cnt) __attribute__((always_inline));
static __inline void outsl(int port, const void *addr, int cnt) __attribute__((always_inline));
static __inline void outl(int port, uint32_t data) __attribute__((always_inline));
static __inline void invlpg(void *addr) __attribute__((always_inline));
static __inline void lidt(void *p) __attribute__((always_inline));
static __inline void lldt(uint16_t sel) __attribute__((always_inline));
static __inline void ltr(uint16_t sel) __attribute__((always_inline));
static __inline void lcr0(uint32_t val) __attribute__((always_inline));
static __inline uint32_t rcr0(void) __attribute__((always_inline));
static __inline uint32_t rcr2(void) __attribute__((always_inline));
static __inline void lcr3(uint32_t val) __attribute__((always_inline));
static __inline uint32_t rcr3(void) __attribute__((always_inline));
static __inline void lcr4(uint32_t val) __attribute__((always_inline));
static __inline uint32_t rcr4(void) __attribute__((always_inline));
static __inline void tlbflush(void) __attribute__((always_inline));
static __inline uint32_t read_eflags(void) __attribute__((always_inline));
static __inline void write_eflags(uint32_t eflags) __attribute__((always_inline));
static __inline uint32_t read_ebp(void) __attribute__((always_inline));
static __inline uint32_t read_esp(void) __attribute__((always_inline));
static __inline void cpuid(uint32_t info, uint32_t *eaxp, uint32_t *ebxp, uint32_t *ecxp, uint32_t *edxp);
static __inline uint32_t cpuid_eax(uint32_t func);
static __inline uint32_t cpuid_ebx(uint32_t func);
static __inline uint32_t cpuid_ecx(uint32_t func);
static __inline uint32_t cpuid_edx(uint32_t func);
static __inline uint64_t read_tsc(void) __attribute__((always_inline));
static __inline void pause(void);
static __inline void write_msr(uint32_t msr, uint64_t val);
static __inline uint64_t read_msr(uint32_t msr);
static __inline void atomic_dec(struct sem *sem);
static __inline void atomic_inc(struct sem *sem);
static __inline uint64_t cmpxch(struct sem *sem, uint64_t v1, uint64_t v2);
static __inline void spin_lock(struct sem *sem);
static __inline void spin_unlock(struct sem *sem);

static __inline void
breakpoint(void)
{
	__asm __volatile("int3");
}

static __inline uint8_t
inb(int port)
{
	uint8_t data;
	__asm __volatile("inb %w1,%0" : "=a" (data) : "d" (port));
	return data;
}

static __inline void
insb(int port, void *addr, int cnt)
{
	__asm __volatile("cld\n\trepne\n\tinsb"			:
			 "=D" (addr), "=c" (cnt)		:
			 "d" (port), "0" (addr), "1" (cnt)	:
			 "memory", "cc");
}

static __inline uint16_t
inw(int port)
{
	uint16_t data;
	__asm __volatile("inw %w1,%0" : "=a" (data) : "d" (port));
	return data;
}

static __inline void
insw(int port, void *addr, int cnt)
{
	__asm __volatile("cld\n\trepne\n\tinsw"			:
			 "=D" (addr), "=c" (cnt)		:
			 "d" (port), "0" (addr), "1" (cnt)	:
			 "memory", "cc");
}

static __inline uint32_t
inl(int port)
{
	uint32_t data;
	__asm __volatile("inl %w1,%0" : "=a" (data) : "d" (port));
	return data;
}

static __inline void
insl(int port, void *addr, int cnt)
{
	__asm __volatile("cld\n\trepne\n\tinsl"			:
			 "=D" (addr), "=c" (cnt)		:
			 "d" (port), "0" (addr), "1" (cnt)	:
			 "memory", "cc");
}

static __inline void
outb(int port, uint8_t data)
{
	__asm __volatile("outb %0,%w1" : : "a" (data), "d" (port));
}

static __inline void
outsb(int port, const void *addr, int cnt)
{
	__asm __volatile("cld\n\trepne\n\toutsb"		:
			 "=S" (addr), "=c" (cnt)		:
			 "d" (port), "0" (addr), "1" (cnt)	:
			 "cc");
}

static __inline void
outw(int port, uint16_t data)
{
	__asm __volatile("outw %0,%w1" : : "a" (data), "d" (port));
}

static __inline void
outsw(int port, const void *addr, int cnt)
{
	__asm __volatile("cld\n\trepne\n\toutsw"		:
			 "=S" (addr), "=c" (cnt)		:
			 "d" (port), "0" (addr), "1" (cnt)	:
			 "cc");
}

static __inline void
outsl(int port, const void *addr, int cnt)
{
	__asm __volatile("cld\n\trepne\n\toutsl"		:
			 "=S" (addr), "=c" (cnt)		:
			 "d" (port), "0" (addr), "1" (cnt)	:
			 "cc");
}

static __inline void
outl(int port, uint32_t data)
{
	__asm __volatile("outl %0,%w1" : : "a" (data), "d" (port));
}

static __inline void 
invlpg(void *addr)
{ 
	__asm __volatile("invlpg (%0)" : : "r" (addr) : "memory");
}  

static __inline void
lidt(void *p)
{
	__asm __volatile("lidt (%0)" : : "r" (p));
}

static __inline void
sidt(void *p)
{
	__asm __volatile("sidt (%0)" : : "r" (p));
}

static __inline void
lldt(uint16_t sel)
{
	__asm __volatile("lldt %0" : : "r" (sel));
}

static __inline void
ltr(uint16_t sel)
{
	__asm __volatile("ltr %0" : : "r" (sel));
}

static __inline void
lcr0(uint32_t val)
{
	__asm __volatile("movl %0,%%cr0" : : "r" (val));
}

static __inline uint32_t
rcr0(void)
{
	uint32_t val;
	__asm __volatile("movl %%cr0,%0" : "=r" (val));
	return val;
}

static __inline uint32_t
rcr2(void)
{
	uint32_t val;
	__asm __volatile("movl %%cr2,%0" : "=r" (val));
	return val;
}

static __inline void
lcr3(uint32_t val)
{
	__asm __volatile("movl %0,%%cr3" : : "r" (val));
}

static __inline uint32_t
rcr3(void)
{
	uint32_t val;
	__asm __volatile("movl %%cr3,%0" : "=r" (val));
	return val;
}

static __inline void
lcr4(uint32_t val)
{
	__asm __volatile("movl %0,%%cr4" : : "r" (val));
}

static __inline uint32_t
rcr4(void)
{
	uint32_t cr4;
	__asm __volatile("movl %%cr4,%0" : "=r" (cr4));
	return cr4;
}

static __inline void
tlbflush(void)
{
	uint32_t cr3;
	__asm __volatile("movl %%cr3,%0" : "=r" (cr3));
	__asm __volatile("movl %0,%%cr3" : : "r" (cr3));
}

static __inline uint32_t
read_eflags(void)
{
        uint32_t eflags;
        __asm __volatile("pushfl; popl %0" : "=r" (eflags));
        return eflags;
}

static __inline void
write_eflags(uint32_t eflags)
{
        __asm __volatile("pushl %0; popfl" : : "r" (eflags));
}

static __inline uint32_t
read_ebp(void)
{
        uint32_t ebp;
        __asm __volatile("movl %%ebp,%0" : "=r" (ebp));
        return ebp;
}

static __inline uint32_t
read_esp(void)
{
        uint32_t esp;
        __asm __volatile("movl %%esp,%0" : "=r" (esp));
        return esp;
}

static __inline void
cpuid(uint32_t info, uint32_t *eaxp, uint32_t *ebxp, uint32_t *ecxp, uint32_t *edxp)
{
	uint32_t eax, ebx, ecx, edx;
	__asm __volatile("cpuid" 
		: "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx)
		: "a" (info));
	if (eaxp)
		*eaxp = eax;
	if (ebxp)
		*ebxp = ebx;
	if (ecxp)
		*ecxp = ecx;
	if (edxp)
		*edxp = edx;
}

static __inline uint32_t
cpuid_eax(uint32_t func)
{
	uint32_t eax;
	cpuid(func, &eax, NULL, NULL, NULL);
	return eax;
}

static __inline uint32_t
cpuid_ebx(uint32_t func)
{
	uint32_t ebx;
	cpuid(func, NULL, &ebx, NULL, NULL);
	return ebx;
}

static __inline uint32_t
cpuid_ecx(uint32_t func)
{
	uint32_t ecx;
	cpuid(func, NULL, NULL, &ecx, NULL);
	return ecx;
}

static __inline uint32_t
cpuid_edx(uint32_t func)
{
	uint32_t edx;
	cpuid(func, NULL, NULL, NULL, &edx);
	return edx;
}

static __inline uint64_t
read_tsc(void)
{
        uint64_t tsc;
        __asm __volatile("rdtsc" : "=A" (tsc));
        return tsc;
}

static __inline void pause()
{
	__asm __volatile("pause"::);
}

static __inline void write_msr(uint32_t msr, uint64_t val)
{
	uint32_t lo = val & 0xffffffff;	
	uint32_t hi = val >> 32;
	__asm __volatile("wrmsr"::"c"(msr), "a"(lo), "d"(hi));
}

static __inline uint64_t read_msr(uint32_t msr)
{
	uint32_t lo, hi;
	__asm __volatile("rdmsr":"=d"(hi), "=a"(lo): "c"(msr));
	return (((uint64_t) lo) | (((uint64_t) hi) << 32));
}

static __inline void atomic_inc(struct sem *sem)
{
	__asm __volatile("lock; incq %0":"+m"(sem->semph)::"cc");
}

static __inline void atomic_dec(struct sem *sem)
{
	__asm __volatile("lock; decq %0":"+m"(sem->semph)::"cc");
}

static __inline uint64_t cmpxch(struct sem *sem, uint64_t v1, uint64_t v2)
{
	__asm __volatile("lock; cmpxchgq %2, %0":"=m"(sem->semph):"A"(v1), "D"(v2):"memory", "%rax", "cc");
	return sem->semph;
}

static __inline void spin_lock(struct sem *sem)
{
	__asm __volatile(
			"\n1: \n\t"
			"lock; decl %0\n\t"
			"jns 3f\n\t"
			"2:\n\t"
			"pause \n\t"
			"cmp $1, %0\n\t"
			"jne 2b\n\t"
			"jmp 1b\n\t"
			"3:\n\t":"=m"(sem->semph)::"memory", "cc");
}

static __inline void spin_unlock(struct sem *sem)
{
	sem->semph = 1;
}

static __inline void disable_mce(void)
{
	uint32_t flag = 0;
	uint64_t msr = 0;
/*	__asm__ __volatile__("cpuid\n\t"
								:"=d"(flag)
								:"a"(0x1)
								:"cc");
	if((flag & 0x4000) != 0){
		lock_cprintf("Machine check architecture detected\n");
	}else{
		return;
	}*/
	__asm __volatile("mov %%cr4, %%rax\n\t"
						"and $0x7bf, %%rax\n\t"
						"mov %%rax, %%cr4\n\t"
						::	);

	msr = read_msr(0x179);
	if((msr & 0x100)!=0){
		write_msr(0x17b, 0);
	}
}
#endif /* !JOS_INC_X86_H */
