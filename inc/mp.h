#ifndef MP_HEADERS
#define MP_HEADERS

#include <inc/types.h>
#include <inc/x86.h>

#define PROC 0x0
#define BUS 0x1
#define IOAPIC 0x2
#define IOINTR 0x3
#define LINTR 0x4


struct mp_fptr* mp_get_fptr(void);
extern struct mp_fptr *gfptr;
extern uint64_t cpuid_offset;



struct mp_fptr //mp floating pointer structure
{
	uint8_t ft_sig[4]; //"_MP_"
	uint32_t tb_addr; //ta phsy addr
	uint8_t lenth; // length of the fptr(in 16 bytes)
	uint8_t sepc;  // mp spec version
	uint8_t check_sum; //all bytes in the fptr must added zero 
	uint8_t mp_feature[5]; // features [0]:zero for mp table present others stands for the default,[1]:0-6 reserverd, bit 7 IMCRP--present or not
}__attribute__((__packed__));

struct mp_conf_header  //mp_configure table header
{
	uint8_t header_sig[4]; //"PCMP"
	uint16_t base_t_length; // base table length in bytes
	uint8_t sepc; //mp table sepc
	uint8_t check_sum; //all bytes must be added to zero
	uint8_t oem_info[20]; //oem info
	uint32_t oem_t_addr; //oem table addr
	uint16_t oem_t_size; //oem table size
	uint16_t count; //entry count
	uint32_t lapicaddr; //lapic map addr
	uint16_t ext_t_length; //extended table length
	uint8_t e_checksum; //extended table checksum
	uint8_t reserved; 
}__attribute__((__packed__));

/*
 * all base table entries are sorted by the entry type in ascending order
 */

struct mp_processer //type 0, length 20bytes
{
	uint8_t type; //0
	uint8_t lapicid;
	uint8_t lapicver;
	uint8_t cpuflags; //bit 0:enabled/disabled, bit 1:BP/AP, others reserverd
	uint8_t signature[4]; //cpu signare: stepping ,model ,family
	uint32_t feature; //refer to the cpuid
	uint32_t reserverd[2]; 
}__attribute__((__packed__));

struct mp_busentry //type 1, length 8bytes
{
	uint8_t type; //1
	uint8_t busid; 
	uint8_t busstr[6];
}__attribute__((__packed__));

struct mp_ioapic  //type 2,  length 8bytes
{
	uint8_t type; //2
	uint8_t ioapicid;
	uint8_t ioapicvers;
	uint8_t ioapicflag; // bit 0: enabled/disabled
	uint32_t mmioapic_addr;
}__attribute__((__packed__));

struct mp_iointr_assign  //type 3, length 8bytes
{
	uint8_t type; //3
	uint8_t intr_type;
	uint8_t io_intrflag; // bit 0-1:polarity of apic I/O, bit 2-3:trigger mode
	uint8_t reserver;
	uint8_t sourcebusid;
	uint8_t sourcebusirq;
	uint8_t desioapicid;
	uint8_t desioapicintn;
}__attribute__((__packed__));

struct mp_locintr_assign  //typ2 4, length 8bytes
{
	uint8_t type; //4
	uint8_t intr_type;
   uint8_t io_intrflag; // bit 0-1:polarity of apic I/O, bit 2-3:trigger mode
	uint8_t reserver;
	uint8_t sourcebusid;
	uint8_t sourcebusirq;
	uint8_t deslapicid;
	uint8_t deslapicintn;
}__attribute__((__packed__));

static uint32_t sum(uint8_t *p, uint32_t size)
{
	uint8_t totl = 0;
	uint32_t i;
	for(i = 0; i < size; i ++)	
		totl += p[i];
	return totl;
}

static void apic_mtrr_init()
{
	write_msr(0x230, 0xfee00000);
	write_msr(0x231, (((((uint64_t) 1)<<36 )- 1 ) & ~(1000 -1)) | 0x800);
	write_msr(0x232, 0xfec00000);
	write_msr(0x233, (((((uint64_t) 1)<<36) - 1 ) & ~(1000 -1)) | 0x800);
}

static struct mp_fptr* mp_search(uint64_t base, uint32_t limit)
{
	uint32_t offset = 0;
	struct mp_fptr *fptr;
	while(offset < limit)
	{
		fptr = (struct mp_fptr*)(base + offset);
		if(fptr->ft_sig[0] == '_' &&fptr->ft_sig[1] == 'M' && 
			fptr->ft_sig[2] == 'P' && fptr->ft_sig[3] == '_')
		{
			if(sum((uint8_t *)fptr, sizeof(struct mp_fptr)) == 0)
			{
				return fptr;
			}
		}
		offset += sizeof(struct mp_fptr);
	}
	return 0;
}
int mp_init(void);
int smp_boot(void);
void dump_ioapic(struct mp_fptr *fptr);
#endif
