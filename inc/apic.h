#ifndef APIC_HEADERS
#define APIC_HEADERS

#include <inc/types.h>
#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/mp.h>

#define NO_SHORTHAND 0x0
#define SELF (0x1<<18)
#define ALL_BUT_SELF (0x2<<18)
#define ALL (0x3<<18)

#define EDGE_TRIG 0x0
#define LEVL_TRIG (0x1<<15)

#define ASSERT (0x1<<14)
#define DE_ASS (0x0)

#define DELIVER_ST (0x1<<12)

#define PHY_DES 0x0
#define LOGI_DES 0x800

#define FIXED 0x0
#define LOWES_PRO (0x1<<8)
#define SMI (0x10<<8)
#define RESERV (0x11<<8)
#define NMI (0x100<<8)
#define INIT (0x101<<8)
#define START_UP (0x110<<8)
#define RESVER (0x111<<8)

#define AP_BOOT_SIG ((uint16_t *)0x100000)

extern uint32_t ioapicid;
extern uint64_t cpuid_offset;
extern struct mp_ioapic *ioapic[16];

struct mp_fptr* mp_get_fptr(void);
void ioapic_init(struct mp_ioapic *ioapic[16], struct mp_fptr *fptr);
uint32_t read_ioapic(uint16_t sel);
void write_ioapic(uint16_t sel, uint32_t data);
void mul_write_ioapic(uint16_t sel, uint32_t data, uint64_t base);
uint32_t mul_read_ioapic(uint16_t sel, uint64_t base);
uint32_t read_lapic(uint32_t offset);
void write_lapic(uint32_t offset, uint32_t data);
int ioapic_tmp_map(uint64_t addr);
int lapic_tmp_map(uint64_t addr);
int enable_lapic(void);
int apic_ipi_init(uint32_t id);
int apic_ipi(uint32_t id, uint32_t irqno);
int boot_ap(uint32_t pa, uint32_t id);
int delay(uint64_t cycles);
uint32_t lapicid(void);
void enable_irq(uint32_t irq, uint32_t cpu);
void ioapic_table(uint64_t addr);
void keyboard_irq_redirect(void);
#endif
