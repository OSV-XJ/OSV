#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/irq.h>
#include <inc/percpu.h>
#include <inc/x86.h>
//#include <inc/intr.h>
#include <inc/apic.h>
#include <inc/svm.h>

#include <kern/console.h>

#define BCDTOBIN(val) ((val) = (((val)&0x0f) + ((val)>>4)*10))

#define CMOS_READ(port) ({ \
									outb(0x70, (port)); \
									delay(10); \
									inb(0x71); \
								})

struct wall_time wtime;

extern struct sem  print_sem;

extern volatile uint16_t crt_pos;

volatile uint16_t tmp_crt = 0;
volatile uint16_t time_crt = 0;
volatile uint64_t index_t = 0;

//extern void writebuff(char c);
void set_handler(void (*f)(),int trapno);

extern void intr_fr_pop(struct interrupt_frame *fr);

uint32_t size_of_intr = 0;

void init_rtc()
{
	get_rtc(&wtime);
}

void time_print()
{
	spin_lock(&print_sem);
	if(time_crt == 0)
		time_crt = crt_pos;
	tmp_crt = crt_pos;
	crt_pos = 0;
	cprintf("%4u:%2u:%2u:%2u:%2u:%2u, cpuid:%d\n", wtime.year, wtime.month, wtime.day, wtime.hour, wtime.minute, wtime.second, lapicid());
	crt_pos = tmp_crt;
	spin_unlock(&print_sem);
}

void set_rtc()
{
	uint8_t val;
	outb(0x70, 0x0b);
	val = inb(0x71);
	while(!(val&0x4))
	{
		val = inb(0x71);
		val = val | 0x2 | 0x4;
		outb(0x71, val);
	}
}

void get_rtc(struct wall_time *time)
{
	uint8_t sec, min, hr, day, wday, month, year;
	do
	{
		outb(0x70, 0x0);
		sec = inb(0x71);
		delay(10);
		outb(0x70, 0x2);
		min = inb(0x71);
		delay(10);
		outb(0x70, 0x4);
		hr = inb(0x71);
		delay(10);
		outb(0x70, 0x6);
		wday = inb(0x71);
		delay(10);
		outb(0x70, 0x7);
		day = inb(0x71);
		delay(10);
		outb(0x70, 0x8);
		month = inb(0x71);
		delay(10);
		outb(0x70, 0x9);
		year = inb(0x71);
	}while(sec != CMOS_READ(0x0));

	BCDTOBIN(sec);
	BCDTOBIN(min);
	BCDTOBIN(hr);
	BCDTOBIN(wday);
	BCDTOBIN(day);
	BCDTOBIN(month);
	BCDTOBIN(year);
	time->second = sec;
	time->minute = min;
	time->hour = hr;
	time->day = day;
	time->wday = wday;
	time->month = month;
	time->year = year + 1900;
	if(time->year < 1970)
		time->year += 100;
}

void load_idt()
{
	idtdesc1.len = 255*16;
	idtdesc1.addr = (uint64_t)idt_table;
	lidt((void *)&idtdesc1);

}

void set_handler (void (*f)(),int trapno)
{
	GATE * p_gate = &idt_table[trapno];
	uint64_t base = (uint64_t)f;
	p_gate->offset_low = (uint16_t)(base & 0xFFFF);
	p_gate->selector = (uint16_t)0x8;
	p_gate->dcount = (uint8_t)0;
	p_gate->attr = (uint8_t)0x8e;
	p_gate->offset_high = (uint32_t)((((uint64_t)base)>>16) & 0xFFFF);
	p_gate->offset_high32 = (uint32_t)((base>>32) & 0xffffffff);
	p_gate->res_offset_high32 = 0;
}


void save_context()
{
	__asm __volatile("pushq %rbp\n"
			"add $0x8,%esp\n"
			"movq %rsp, %rbp\n"
			"sub $0x8,%esp");
}

void recover_context()
{
	__asm __volatile(
			"leave\n"
			"iretq");
}


void init_8259A(void)
{
	outb(0x20,0x11);
	__asm __volatile("nop\nnop\nnop"::);
	outb(0xA0,0x11);
	__asm __volatile("nop\nnop\nnop"::);
	outb(0x21,0x20);
	__asm __volatile("nop\nnop\nnop"::);
	outb(0xA1,0x28);
	__asm __volatile("nop\nnop\nnop"::);

	outb(0x21,0x04);
	__asm __volatile("nop\nnop\nnop"::);
	outb(0xA1,0x02);
	__asm __volatile("nop\nnop\nnop"::);

	outb(0x21,0x03);
	__asm __volatile("nop\nnop\nnop"::);
	outb(0xA1,0x03);
	__asm __volatile("nop\nnop\nnop"::);
	outb(0xA1,0xFF);
	__asm __volatile("nop\nnop\nnop"::);
	outb(0x21,0xFF);
	__asm __volatile("nop\nnop\nnop"::);

	cprintf("8259A Init has finished\n");

}

void ack_8259a(int irq)
{
	if(irq & 8)
	{
		inb(0xa1);
		outb(0xA1,0xFF);
		outb(0xa0, 0x60 + (irq & 7));
		outb(0x20, 0x62);
	}
	else
	{
		inb(0x21);
		outb(0x21, 0xFF);
		outb(0x20, 0x60 + irq);
	}
}

void spurious_irq(int irq)
{
	cprintf("spurious_irq: ");
	cprintf("%x",irq&0xff);
	cprintf("\n");
}

void pit_init(void)
{
	outb(0x43, 0x34);
	delay(10);
	outb(0x40, (uint8_t)((1193182/10)%256));
	delay(10);

	outb(0x40, (uint8_t)((1193182/10)/256));
	delay(10);
}

//void intr_handler(struct interrupt_frame *frame, uint64_t intrno)
void intr_handler(struct interrupt_frame *frame, uint64_t intrno, uint64_t orig_rsp)
{
	uint8_t test;
	uint64_t err;

	err = frame->error_code;
	uint32_t *tmp_ins;
lock_cprintf("In %s, intrno:%ld, rip:%lx, Proc:%d\n",
		__func__, intrno, frame->orig_rip, read_lapic(0x20)>>24);
	switch(intrno){
		case 2:	//NMI interrupt
			break;

		case 33:
			kbd_intr();
			break;
		case 30:
			lock_cprintf("sx intr\n");
			break;
		case 34:
		case 32:
			get_rtc(&wtime);
			time_print();
			index_t ++;
			break;
		case 0x30:
			{
				uint32_t queued = 0;
				int i;
				for(i=7; i>=0; i--){
					queued = read_lapic(0x200+ i*0x10);
					if(queued){
						lock_cprintf("i:%d, queued:%x\n", i, queued);
					}
					queued = 0;
				}
			}
			break;
		case 0xef:
			lock_cprintf("here in the local apic interrupt\n");
			break;
		default:
			write_lapic(0xb0, 0x0);
			tmp_ins = (uint32_t *)(frame->orig_rip+1);

			lock_cprintf("\nIntr %ld, Proc:%d, err_code:0x%lx, rip:%lx-%x, old_CS:%lx, RSP:%lx, SS:%lx\n",
					intrno, read_lapic(0x20)>>24, frame->error_code, frame->orig_rip, *tmp_ins,
					frame->orig_cs, frame->orig_rsp, frame->orig_ss);
			//		while(1);

	}
	struct vmcb *vmcb1;
	vmcb1 = &vmcb_test[lapicid()];
	irq_inject(vmcb1, intrno);
	write_lapic(0xb0, 0x0);

	//	lock_cprintf("Intr_handler return, %d\n", intrno);
	if(intrno == 0x33){
		uint32_t queued = 0;
		int i;
		for(i=7; i>=0; i--){
			queued = read_lapic(0x200 + i*0x10);
			if(queued){
				lock_cprintf("Again i:%d, queued:%x\n", i, queued);
			}
			queued = 0;
		}
	}

	intr_fr_pop(frame);
}

void keyboard_int()
{
	save_context();
	//writebuff(inb(0x60));
	recover_context();
}

void set_exception()
{
	int i = 0;

	set_handler((void*)intr_noec(0), 0);
	set_handler((void*)intr_noec(1), 1);
	set_handler((void*)intr_noec(2), 2);
	set_handler((void*)intr_noec(3), 3);
	set_handler((void*)intr_noec(4), 4);
	set_handler((void*)intr_noec(5), 5);
	set_handler((void*)intr_noec(6), 6);
	set_handler((void*)intr_noec(7), 7);
	set_handler((void*)intr_ec(8), 8);
	set_handler((void*)intr_ec(9), 9);
	set_handler((void*)intr_ec(10), 10);
	set_handler((void*)intr_ec(11), 11);
	set_handler((void*)intr_ec(12), 12);
	set_handler((void*)intr_ec(13), 13);
	set_handler((void*)intr_ec(14), 14);
	set_handler((void*)intr_ec(15), 15);
	set_handler((void*)intr_noec(30), 30);
	set_handler((void*)intr_noec(31), 31);

	for(i=16; i<30; i++)
		set_handler((void*)intr_noec(i), i);
}

void set_interrupt()
{
	for(int i = 32; i <= 255; i ++)
	{
		set_handler((void *)intr_noec(i), i);
	}
}

