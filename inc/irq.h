/* See COPYRIGHT for copyright information. */
#ifndef HKEY_IRQ_HEADER
#define HKEY_IRQ_HEADER

#include <inc/types.h>
#include <inc/svm.h>

struct interrupt_frame
{
	uint64_t fr_rax;
	uint64_t fr_rbx;
	uint64_t fr_rcx;
	uint64_t fr_rdx;
	uint64_t fr_rdi;
	uint64_t fr_rsi;
	uint64_t fr_rbp;
	uint64_t fr_r8;
	uint64_t fr_r9;
	uint64_t fr_r10;
	uint64_t fr_r11;
	uint64_t fr_r12;
	uint64_t fr_r13;
	uint64_t fr_r14;
	uint64_t fr_r15;	
//	uint64_t intr_fr_rip;
	uint64_t error_code;
	uint64_t orig_rip;
	uint64_t orig_cs;
	uint64_t rflags;
	uint64_t orig_rsp;
	uint64_t orig_ss;
};

typedef struct  idt_gate
{
	uint16_t offset_low;
	uint16_t selector;
	uint8_t dcount;
	uint8_t attr;
	uint16_t offset_high;
	uint32_t offset_high32;
	uint32_t res_offset_high32;
}__attribute__((__packed__)) GATE;

typedef struct __attribute__((__packed__)) idt_desc
{
	uint16_t len;
	uint64_t addr;
}DESC;

//GATE idt_table[256] __attribute__((aligned (4096), section(".data")));
GATE idt_table[256];

DESC idtdesc1;

struct wall_time
{
	uint8_t second;
	uint8_t minute;
	uint8_t hour;
	uint8_t wday;
	uint8_t day;
	uint8_t month;
	uint32_t year;
};

extern char intr_noec_0[];
extern char intr_ec_0[];
extern char intr_end_0[];

#define intr_noec(intrno) (intr_noec_0 + ((uint64_t)intr_end_0 - (uint64_t)intr_noec_0)*intrno)
#define intr_ec(intrno) (intr_noec(intrno) + ((uint64_t)intr_ec_0 - (uint64_t)intr_noec_0))


void init_8259A(void);
void set_exception();
void set_interrupt();
void load_idt();
void keyboard_init(void);
void set_handler (void (*f)(),int trapno);
void save_context();
void pit_init();
void recover_context();
//void intr_handler(struct interrupt_frame *frame, uint64_t intrno);
void intr_handler(struct interrupt_frame *frame, uint64_t intrno, uint64_t orig_rip);

void init_rtc();
void set_rtc();
void get_rtc(struct wall_time *time);
void time_print();

extern struct vmcb vmcb_test[32];
#endif
