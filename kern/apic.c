#include <inc/types.h>
#include <inc/apic.h>
#include <inc/pmap.h>
#include <inc/x86.h>



uint32_t read_ioapic(uint16_t sel)
{
    uint32_t val;
    *((volatile uint32_t *) 0xfec00000) = sel;
    val = *((volatile uint32_t*)0xfec00010);
    return val;
}

void write_ioapic(uint16_t sel, uint32_t data)
{
    *((volatile uint32_t *) 0xfec00000) = sel;
    *((volatile uint32_t *) 0xfec00010) = data;
}

uint32_t read_lapic(uint32_t offset)
{
    return *(volatile uint32_t *)((uint64_t)0xfee00000 + offset);
}

void write_lapic(uint32_t offset, uint32_t data)
{
	uint64_t apic_base = 0;

	apic_base = read_msr(0x1b);
	__asm__ __volatile__("mfence":::"memory");
	if((apic_base & ~0xfff)!= 0xfee00000){
		while(1)
			lock_cprintf("LAPIC Base changed %#lx\n", apic_base);
	}

	*(volatile uint32_t *)((uint64_t)0xfee00000+ offset) = data;
	read_lapic(offset);
}

static int disable_mcr(struct mp_fptr *fptr)
{
	if((fptr->mp_feature[1] & 0x80) == 0)	//IMCR is not present
		return -1;
	outb(0x22, 0x70);
	outb(0x23, 0x1);
	return 1;
}

uint32_t mul_read_ioapic(uint16_t sel, uint64_t base)
{
	uint32_t val;
	*((volatile uint32_t *) base) = sel;
	val = *((volatile uint32_t*)(base + 0x10));
	return val;
}

void mul_write_ioapic(uint16_t sel, uint32_t data, uint64_t base)
{
	*((volatile uint8_t *) base) = sel;
	*((volatile uint32_t *) (base + 0x10)) = data;
}

void ioapic_table(uint64_t addr)
{
	uint32_t info ;
	uint64_t intr_red;
	int entries, i;
	entries = (mul_read_ioapic(1, addr)>>16) & 0xff;
	lock_cprintf("IOAPIC id:%d version:%d, entries:%d\n", (mul_read_ioapic(0, addr)>>24) & 0xf,
			mul_read_ioapic(1, addr) & 0xff, entries);
	for(i=0; i<entries; i++){
		intr_red = mul_read_ioapic(0x11+2*i, addr);
		intr_red = intr_red << 32;
		intr_red += mul_read_ioapic(0x10+2*i, addr);
		lock_cprintf("%lx\t", intr_red);
	}
	lock_cprintf("\n");
}


void dump_ioapic(struct mp_fptr *fptr)
{
	struct mp_conf_header *mp_header;
	struct mp_ioapic *ioapic;
	struct mp_iointr_assign *iointr;
	struct mp_locintr_assign *lintr;

	const char *INT_TYPE[4] = {
		"INT",
		"NMI",
		"SMI",
		"ExtINT"
	};

	mp_header = (struct mp_conf_header *)((uint64_t)  fptr->tb_addr );
	uint8_t *p = (uint8_t *)(mp_header + 1);
	int index = 0;

	for(; p < ((uint8_t *)mp_header + mp_header->base_t_length);)
	{
		switch(*p)
		{
			case PROC:
				p += 20;
				continue;
			case BUS:
				p += 8;
				continue;
			case IOAPIC:
				ioapic = (struct mp_ioapic *) p;
				cprintf("IOAPIC[%d], version %d,  mem mapped addr:0x%x, <%d>\n", ioapic->ioapicid,
						ioapic->ioapicvers, ioapic->mmioapic_addr,ioapic->ioapicflag); 
				p += 8;
				continue;
			case IOINTR:
				iointr = (struct mp_iointr_assign *) p;
				lock_cprintf("IO intr assign [%s], Src bus:%d, irq:%d, Des IOAPIC:%d, INTIN#%d\n",
						INT_TYPE[iointr->intr_type], iointr->sourcebusid,
						iointr->sourcebusirq, iointr->desioapicid, iointr->desioapicintn);
				p += 8;
				continue;
			case LINTR:
				lintr = (struct mp_locintr_assign *) p;
				lock_cprintf("Local intr assign [%s], Src bus:%d, irq:%d, Des LAPIC:%d, LINTIN#%d\n",
						INT_TYPE[lintr->intr_type], lintr->sourcebusid,
						lintr->sourcebusirq, lintr->deslapicid, lintr->deslapicintn);
				p += 8;
				continue;
			default:
				cprintf("error! Unkown mp conf type\n");
				return;
		}
	}
}

void ioapic_init(struct mp_ioapic *ioapic[8], struct mp_fptr *fptr)
{
	uint32_t ioapic_base_addr, data;
	uint8_t ioapicid, intr_type;

	struct mp_conf_header *mp_header;
	struct mp_iointr_assign *iointr;

	mp_header = (struct mp_conf_header *)((uint64_t)  fptr->tb_addr );
	uint8_t *p = (uint8_t *)(mp_header + 1);

	disable_mcr(fptr);

	uint32_t LINT0;
	LINT0 = read_lapic(0x350);
	lock_cprintf("LINT0 :%x\n", LINT0);
	LINT0 &= ~(1<<16);
	write_lapic(0x350, LINT0);


	for(; p < ((uint8_t *)mp_header + mp_header->base_t_length);)
	{
		switch(*p)
		{
			case PROC:
				p += 20;
				continue;
			case IOINTR:
				iointr = (struct mp_iointr_assign *) p;
				ioapicid = iointr->desioapicid;
				ioapic_base_addr = ioapic[ioapicid]->mmioapic_addr;
				if(ioapicid == 8){
					if(iointr->intr_type == 0 && iointr->desioapicintn==2){
						data = 0 << 24;	// destinatioin
						mul_write_ioapic(0x11 + 2 * iointr->desioapicintn,
								data, ioapic_base_addr);

						data = iointr->desioapicintn;
						/* bit[8-11] is 0, physical mode, Fixed delivery */
						if((iointr->io_intrflag & 0x3)==0x3){ //Polarity is low
							data |= 1<<13;
						}
						if((iointr->io_intrflag & 0xc)==0xc){ //Level triggered
							data |= 1<<15;
						}

						data |= 0<<16;	// enable interrupt

					data += 0x30;
						mul_write_ioapic(0x10 + 2 * iointr->desioapicintn,
								data, ioapic_base_addr);
						lock_cprintf("IOAPIC base addr:%x\n", ioapic_base_addr);
					}
				}
				p += 8;
				continue;
			case BUS:
			case IOAPIC:
			case LINTR:
				p += 8;
				continue;
			default:
				lock_cprintf("Unknown entry\n");
				return;
		}
	}
}

void keyboard_irq_redirect(void)
{
	int i = 0;
	uint32_t low, high, entries;
	uint64_t ioapic_base_addr;

	ioapic_base_addr = ioapic[0]->mmioapic_addr;
	entries = (mul_read_ioapic(1, ioapic_base_addr) >> 16) & 0xff;
	/*	for(; i<entries; i++){
		low = mul_read_ioapic(0x10+2*i, ioapic_base_addr);
		if((low & 0xff) == 0x31){	// This is a keyborad interrupt entry
		high = mul_read_ioapic(0x11+2*i, ioapic_base_addr);
		high &= 0xffffff;
		high |= 0x44000000;
		mul_write_ioapic(0x11+2*i, high, ioapic_base_addr);
		low |= 0x10000;
		mul_write_ioapic(0x10+2*i, low, ioapic_base_addr);
		lock_cprintf("Keyboard irq redirected successed, %x, %x",
		inb(0x21), inb(0xa1));
		break;
		}
		}*/
	low = mul_read_ioapic(0x10+2*19, ioapic_base_addr);
	high = mul_read_ioapic(0x11+2*19, ioapic_base_addr);
	high &= 0xffffff;
	high |= 0x44000000;
	low |= 0x10000;
	mul_write_ioapic(0x10+2*19, low, ioapic_base_addr);
	mul_write_ioapic(0x11+2*19, high, ioapic_base_addr);

	/*	low = mul_read_ioapic(0x10+2*1, ioapic_base_addr);
		high = mul_read_ioapic(0x11+2*1, ioapic_base_addr);
		high &= 0xffffff;
		high |= 0x44000000;
	//	low |= 0x10000;
	//	mul_write_ioapic(0x10+2*1, low, ioapic_base_addr);
	mul_write_ioapic(0x11+2*1, high, ioapic_base_addr);

	low = mul_read_ioapic(0x10+2*12, ioapic_base_addr);
	high = mul_read_ioapic(0x11+2*12, ioapic_base_addr);
	high &= 0xffffff;
	high |= 0x44000000;
	//	low |= 0x10000;
	//	mul_write_ioapic(0x10+2*12, low, ioapic_base_addr);
	mul_write_ioapic(0x11+2*12, high, ioapic_base_addr);

	lock_cprintf("Keyboard irq redirected successed, %x, %x",
	inb(0x21), inb(0xa1));

	outb(0xff, 0x21);
	outb(0xff, 0xa1);
	lock_cprintf(", %x, %x\n", inb(0x21), inb(0xa1));*/
}

void enable_irq(uint32_t irq, uint32_t cpu)
{
	write_ioapic((0x10 + (irq-32)*2), irq);
	write_ioapic((0x10 + (irq-32)*2 + 1), cpu<<24);
}

int enable_lapic(void)
{
	uint32_t i = read_lapic(0xf0);
	uint64_t apic_base_reg;

	apic_base_reg = read_msr(0x1B);
	lock_cprintf("APIC Base Address Register:%lx\n", apic_base_reg);
	if(i & 0x100)
		lock_cprintf("lapic is enabled\n");
	else
	{
		i |= 0x100;
		write_lapic(0xf0, i);
	}

	write_lapic(0xf0, 0x100 | (32 + 31));
	write_lapic(0x370, 32+19);

	write_lapic(0x280, 0);
	write_lapic(0x280, 0);

	write_lapic(0x310, 0);
	write_lapic(0x300, 0x80000|0x500|0x8000|0x0);
	while(read_lapic(0x300) & 0x1000);

	write_lapic(0xb0, 0);
	write_lapic(0x80, 0);

	return 1;
}

int apic_wait(void)
{
	uint64_t i = 10000000;
	while(read_lapic(0x300)&DELIVER_ST)
	{
		delay(10);
		i --;
		if(i == 0)
		{
			lock_cprintf("apic so busy\n");
			return -1;
		}
	}
	return 0;
}

int delay(uint64_t cycles)
{
	uint64_t now;
	now = read_tsc();
	while((read_tsc() - now) < cycles)
		pause();
	return 1;
}

int apic_ipi(uint32_t id, uint32_t irq)
{
	write_lapic(0x310, id<<24);
	write_lapic(0x300, irq | FIXED | EDGE_TRIG | DE_ASS);
	apic_wait();
	if(read_lapic(0x300) & DELIVER_ST)
		return -1;
	else
		return 0;
}

int apic_ipi_init(uint32_t id)
{
	write_lapic(0x310, id<<24);
	write_lapic(0x300, id | 0x500 | 0x8000 | 0x4000);
	apic_wait();
	delay(1500000);
	write_lapic(0x300, id | 0x500 | 0x8000 | 0x0);
	apic_wait();
	if(read_lapic(0x300) & DELIVER_ST)
		lock_cprintf("deliver failed\n");
	return 0;
}

int boot_ap(uint32_t pa, uint32_t id)
{
	outb(0x70, 0xf);
	outb(0x71, 0x0a);

	uint16_t *warm_setv = (uint16_t*)0x467;
	warm_setv[0] = 0;
	warm_setv[1] = pa >> 4;	
	apic_ipi_init(id);
	delay(900000);
	while(read_lapic(0x300) & DELIVER_ST);

	write_lapic(0x310, id<<24);
	write_lapic(0x300, 0x600 | (pa>>12));
	apic_wait();
	while(read_lapic(0x300) & DELIVER_ST);

	delay(300000);

	apic_wait();
	write_lapic(0x310, id<<24);
	write_lapic(0x300, 0x600 | (pa>>12));
	apic_wait();
	while(read_lapic(0x300) & DELIVER_ST);
	lock_cprintf("Before %s return, id:%d\n", __func__, id);
	delay(300000);
	while(*AP_BOOT_SIG != 0xdcba)
		__asm__ __volatile__("pause");

	return 1;
}

uint32_t lapicid(void)
{
	extern uint8_t lapicid_to_index[256];
	return (lapicid_to_index[(read_lapic(0x20)>>24)]);
}
