// Simple implementation of cprintf console output for the kernel,
// based on printfmt() and the kernel console's cputchar().

#include <inc/types.h>
#include <inc/stdio.h>
#include <stdarg.h>
#include <inc/x86.h>
#include <inc/vmmcons.h>

struct sem  print_sem;
struct sem *vmm_sem;

buff_info *info = 0;

static void
putch(int ch, int *cnt)
{
	cputchar(ch);
	if(info)
		vmmputchar(ch);
	*cnt++;
}

int
vcprintf(const char *fmt, va_list ap)
{
	int cnt = 0;

	vprintfmt((void*)putch, &cnt, fmt, ap);
	return cnt;
}

int
cprintf(const char *fmt, ...)
{
	va_list ap;
	int cnt;

	va_start(ap, fmt);
	cnt = vcprintf(fmt, ap);
	va_end(ap);
	
	return cnt;
}

int lock_cprintf(const char *fmt, ...)
{
	spin_lock(&print_sem);
	if(info)
	{
		spin_lock(vmm_sem);
		info->cur_start = info->cur_pos;
	}
    
	va_list ap;
	int cnt;

	va_start(ap, fmt);
	cnt = vcprintf(fmt, ap);
	va_end(ap);
	if(info)
	{
		if(info->over_flow && info->start <= info->cur_pos)
			info->start = info->cur_start;
		spin_unlock(vmm_sem);
	}
    	
	spin_unlock(&print_sem);
    
	return cnt;
}

struct debug_info_buf{
	char buf[4096*8-5];
	char sw;
	uint32_t num;
};
struct debug_info_buf debug_info_buf __attribute__((aligned(4096)));
struct sem debug_sem;

int debug_printf(const char *fmt, ...)
{
	int cnt = 0; 
	va_list ap;
	if(debug_info_buf.sw != 'e'){
		return 0;
	}
	spin_lock(&debug_sem);
	va_start(ap, fmt);
	cnt = vsnprintf(debug_info_buf.buf+debug_info_buf.num, 4090, fmt, ap); 
	va_end(ap);
	debug_info_buf.num = (debug_info_buf.num + cnt)%(7*4096);
	spin_unlock(&debug_sem);
	return 0;
}
extern void * memcpy(void *dst0, const void *src0, size_t length);
int debug_printf_init(struct generl_regs *regs)
{
	regs->rbx = (uint64_t)&debug_info_buf;
	debug_info_buf.sw = 'e';
	debug_info_buf.num = 0;
	debug_sem.semph = 1;
/*	lock_cprintf("This is in debug_printf_init function, %p, %p\n", &debug_info_buf, regs->rbx);
	lock_cprintf("size if %x\n", sizeof(struct debug_info_buf));*/
//	memcpy(debug_info_buf.buf, "haohaoxuexi, tian tian xiang shang", 30);
//	debug_info_buf.num = 30;
	return 0;
}
