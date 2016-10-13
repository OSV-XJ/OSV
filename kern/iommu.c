#include <inc/iommu.h>
#include <inc/pci.h>

struct dev_table device_table[MAX_DEV_ID] __attribute__((aligned (4096), section(".data")));
uint32_t intr_remap_tab[1<<11] __attribute__((aligned(4096), section(".data")));

unsigned char cmdbuf[MAX_IOMMU][IOMMU_BUF_SIZE] __attribute__((aligned(4096), section(".data")));
unsigned char eventbuf[MAX_IOMMU][IOMMU_BUF_SIZE] __attribute__((aligned(4096), section(".data")));
volatile uint32_t *mmio_base[MAX_IOMMU];

struct device_entry_4_byte{
	uint8_t type;
	uint16_t deviceid;
	uint8_t data;
}__attribute__((__packed__));

struct device_entry_8_byte{
	uint8_t type;
	uint16_t device_id_a;
	uint8_t data;
	union{
		uint32_t extended_data;
		struct{
			uint8_t handle;
			uint16_t device_id_b;
			uint8_t variety;
		}__attribute__((__packed__)) id;
	}upper_bytes;
}__attribute__((__packed__));

void enqueue_iommu_cmd(int cmd, uint16_t dev_id, int iommu_id)
{
	struct iommu_command *iommu_com_pr;
	volatile uint32_t tail, head;

	head = *(mmio_base[iommu_id] + 0x800) >> 4;
	tail = *(mmio_base[iommu_id] + 0x802) >> 4;
	while((tail+1)%512 == head){
		head = *(mmio_base[iommu_id] + 0x800) >> 4;
	}

	iommu_com_pr = (struct iommu_command *)
			(cmdbuf + tail*16);
	switch(cmd){
		case 1:	// complete wait
			iommu_com_pr->first_op_low = 0;
			iommu_com_pr->first_op_high = 1UL<<28;
			break;
		case 2: // inv table entry
			iommu_com_pr->first_op_low = dev_id;	
			iommu_com_pr->first_op_high = 2UL<<28;
			break;
		case 3:	// inv all IOTLB
			iommu_com_pr->first_op_high = 3UL<<28 | dev_id;
			iommu_com_pr->second_op_low = ~0xfff | 3;
			iommu_com_pr->second_op_high = 0x7fffffff;
			break;
		case 4:
			break;
		case 5:
			break;
	}
	
	tail = (tail + 1) % 512;
	*(mmio_base[iommu_id] + 0x802) = tail << 4;
}

/* This function should be invoked AFTER page table setup
 *
 * And now, we make IOMMU sharing page table with
 * AMD CPU's Nested Page Table (Domain 1)
 *
 */

void enable_DMA_remapping(uint16_t device_id, uint16_t dev_end, uint64_t npt_pml4e)
{
/*		device_table[device_id].lint1pass = 1;
		device_table[device_id].lint0pass = 1;
		device_table[device_id].intctl = 2;
		device_table[device_id].nmipass = 1;
		device_table[device_id].einitpass = 1;
		device_table[device_id].initpass = 1;
		device_table[device_id].intr_table = (uint64_t)intr_remap_tab >>6;
		device_table[device_id].inttablen = 11;
		device_table[device_id].ig = 0;
		device_table[device_id].iv = 0; */
	if(device_id>dev_end){
		lock_cprintf("Device error\n");
		while(1);
	}
	if(dev_end > MAX_DEV_ID){
		lock_cprintf("OSV Only support device id to 0x%x\n", MAX_DEV_ID);
		while(1);
	}
	for(;device_id<=dev_end; device_id++){
		if(!device_table[device_id].v){
			device_table[device_id].sysmgt = 3;
			device_table[device_id].ex = 0;
			device_table[device_id].sd = 0;
			device_table[device_id].cache = 0;
			device_table[device_id].ioctl = 1;
			device_table[device_id].sa = 0;
			device_table[device_id].se = 1;
			device_table[device_id].i = 1;
			device_table[device_id].domainid = 1;
			device_table[device_id].ir = 1;
			device_table[device_id].iw = 1;
			device_table[device_id].page_table = npt_pml4e >> 12;
			device_table[device_id].mode = 4;
			device_table[device_id].tv = 1;
			device_table[device_id].v = 1;
		}

	}
}


void iommu_init(struct ivrs *ivrs)
{
	struct ivhd_head *ivhd;
	struct ivmd_head *ivmd;
	struct device_entry_4_byte * entry_4_byte;
	struct device_entry_8_byte * entry_8_byte;
	uint32_t base_low, base_high;
	uint32_t i,j, device_entry_index, iommu_nums;
	uint16_t dev_start = 0, dev_end = 0;
	extern char npt[];
	extern int iommu_inited;
	uint64_t tmp;

	if(ivrs == NULL){
		lock_cprintf("IVRS table is not presented\n");
		return;
	}

	memset(device_table, 0, 1024*1024*2);

/*	lock_cprintf("IVRS revision:%d, length: %d bytes, IVinfo: %x\n",
			ivrs->header.revision, ivrs->header.length, ivrs->ivinfo);*/
	j = ivrs->header.length;
	iommu_nums = 0;
	for(i=48; i<j; ){
		ivhd = (struct ivhd_head *)((uint64_t)ivrs + i);
		switch(ivhd->type){
			case 0x10:	//IVHD Block
				/*				lock_cprintf("Find a new IOMMU, flags:%x, IVHD length: %d\n",
								ivhd->flags, ivhd->length);
								lock_cprintf("Device ID: %x, Capability Offset: %x, PCI Segment Group: %x, IOMMU Info:%x\n",
								ivhd->dev_id, ivhd->cap_offset, ivhd->pci_group, ivhd->iommu_info);
								get_pci_dw(ivhd->dev_id, ivhd->cap_offset + 0x4, &base_low);
								get_pci_dw(ivhd->dev_id, ivhd->cap_offset + 0x8, &base_high);
								lock_cprintf("IVHD IOMMU Base Address:%lx, PCI Space get Base Address: %lx\n",
								ivhd->iommu_base, ((uint64_t)base_high)<<32 | base_low);
								get_pci_dw(ivhd->dev_id, ivhd->cap_offset + 0x0, &base_low);
								lock_cprintf("IVHD Capability Reg 0: %x\n", base_low);*/
				iommu_nums++;
				if(iommu_nums > MAX_IOMMU){
					lock_cprintf("There are too many IOMMU, OSV only support %d IOMMUs\n", MAX_IOMMU);
					while(1);
				}

				/* 
				 * Set the Device Table Base Address Register (MMIO Offset: 0)
				 * 12 ------ 51 DevTabBase
				 * 0 -------- 8 Table size (4K)
				 */
				mmio_base[iommu_nums-1] = (uint32_t *)(ivhd->iommu_base);	
				base_low = (uint64_t)device_table & 0xfffff000;
				base_low |= 0x1ff;
				*mmio_base[iommu_nums-1] = base_low;
				base_high = ((uint64_t)device_table >> 32) & 0xfffff;
				*(mmio_base[iommu_nums-1]+1) = base_high;

				/* Set IOMMU Command Buffer Base Address Register */
				tmp = (uint64_t)cmdbuf[iommu_nums-1];
				tmp &= 0xffffffffff000;	// base address
				tmp |= 9UL<<56;	//buffer length, 512 entries
				*(mmio_base[iommu_nums-1] + 2) = tmp & 0xffffffff;
				*(mmio_base[iommu_nums-1] + 3) = tmp >> 32;

				/* Set IOMMU Event Log Base Address Register */
				tmp = (uint64_t)eventbuf;
				tmp &= 0xffffffffff000;	// base address
				tmp |= 9UL<<56;			// buffer length, 512 entries
				*(mmio_base[iommu_nums-1] + 4) = tmp & 0xffffffff;
				*(mmio_base[iommu_nums-1] + 5) = tmp >> 32;




				// IOMMU Exclusion Register, Disable
				*(mmio_base[iommu_nums-1] + 8) = 0;	
				tmp = (1<<12) + (1<<11) + (1<<10) + (1<<9) + (1<<8) + (0<<5) + (0<<4) +
					(0<<3) + (1<<2) + (0<<1) + 1;
				*(mmio_base[iommu_nums-1] + 6) = tmp;	// Enable IOMMU device 

				device_entry_index = sizeof(struct ivhd_head);
				for(;device_entry_index < ivhd->length;){
					switch(*(uint8_t *)((uint64_t)ivhd+device_entry_index)>>6){
						case 0:
							entry_4_byte = (struct device_entry_4_byte *)((uint64_t)ivhd+device_entry_index);
							switch(entry_4_byte->type){
								case 0:
									break;
								case 1:
									lock_cprintf("Setting to all devices\n");
									break;
								case 2:
									enable_DMA_remapping(entry_4_byte->deviceid, entry_4_byte->deviceid, (uint64_t)npt);
									break;
								case 3:
									/*			lock_cprintf("Setting to a range devices, start:%x\n",
												entry_4_byte->deviceid);*/
									dev_start = entry_4_byte->deviceid;
									break;
								case 4:
									dev_end = entry_4_byte->deviceid;
									enable_DMA_remapping(dev_start, dev_end, (uint64_t)npt);
									break;
								default:
									break;
							}
							device_entry_index += 4;
							break;
						case 1:
							entry_8_byte = (struct device_entry_8_byte *)((uint64_t)ivhd+device_entry_index);
							switch(entry_8_byte->type){
								case 64:
									break;
								case 65:
									break;
								case 66:
									enable_DMA_remapping(entry_8_byte->device_id_a,
											entry_8_byte->device_id_a, (uint64_t)npt);
									break;
								case 67:
									dev_start = entry_8_byte->device_id_a;
									break;
								case 68:
								case 69:
									break;
								case 70:
									lock_cprintf("Extended select entry, 8 Byte\n");
									break;
								case 71:
									dev_start = entry_8_byte->device_id_a;
									break;
								case 72:
									lock_cprintf("This is special device, 8 Byte\n");
									break;
								default:
									break;
							}
							device_entry_index += 8;
							break;
						case 2:
							lock_cprintf("This is a 16 byte IVHD device entry");
							device_entry_index += 16;
							break; 
						case 3:
							lock_cprintf("This is a 32 byte IVHD device entry");
							device_entry_index += 32;
							break;
					}
				}
//				lock_cprintf("IVHD header length: %d\n", sizeof(struct ivhd_head));
				i += ivhd->length;
				break;
			case 0x20:	//IVMD Block, all peripherals
				ivmd = (struct ivmd_head *)((uint64_t)ivrs + i);
				lock_cprintf("Find a new IVMD, type: 0x20, flags: %x, start_addr: %lx\n",
						ivmd->flags, ivmd->ivmd_start_addr);
				i += sizeof(struct ivmd_head);
				break;
			case 0x21:	//IVMD Block, specified peripheral
				ivmd = (struct ivmd_head *)((uint64_t)ivrs + i);
				lock_cprintf("Find a new IVMD, type: 0x21, flags: %x, start_addr: %lx\n",
						ivmd->flags, ivmd->ivmd_start_addr);
				i += sizeof(struct ivmd_head);
				break;
			case 0x22:	//IVMD Block, peripheral range
				ivmd = (struct ivmd_head *)((uint64_t)ivrs + i);
				lock_cprintf("Find a new IVMD, type: 0x22, flags: %x, start_addr: %lx\n",
						ivmd->flags, ivmd->ivmd_start_addr);
				i += sizeof(struct ivmd_head);
				break;
			default:
				i++;
				break;
		}
	}
#if 0
	{
		uint16_t device_id;
		device_id = (0<<8) | (0x11<<3) | 0;
		enable_DMA_remapping(device_id, device_id, (uint64_t)npt);
		device_id = (0<<8) | (0x12<<3) | 0;
		enable_DMA_remapping(device_id, device_id, (uint64_t)npt);
		device_id = (0<<8) | (0x12<<3) | 1;
		enable_DMA_remapping(device_id, device_id, (uint64_t)npt);
		device_id = (0<<8) | (0x12<<3) | 2;
		enable_DMA_remapping(device_id, device_id, (uint64_t)npt);
		device_id = (0<<8) | (0x13<<3) | 0;
		enable_DMA_remapping(device_id, device_id, (uint64_t)npt);
		device_id = (0<<8) | (0x13<<3) | 1;
		enable_DMA_remapping(device_id, device_id, (uint64_t)npt);
		device_id = (0<<8) | (0x13<<3) | 2;
		enable_DMA_remapping(device_id, device_id, (uint64_t)npt);
		device_id = (1<<8) | (0<<3) | 0;
		enable_DMA_remapping(device_id, device_id, (uint64_t)npt);
		device_id = (1<<8) | (0<<3) | 1;
		enable_DMA_remapping(device_id, device_id, (uint64_t)npt);
		device_id = (2<<8) | (0<<3) | 0;
		enable_DMA_remapping(device_id, device_id, (uint64_t)npt);
		device_id = (2<<8) | (0<<3) | 1;
		enable_DMA_remapping(device_id, device_id, (uint64_t)npt);
		device_id = (5<<8) | (0<<3) | 0;
		enable_DMA_remapping(device_id, device_id, (uint64_t)npt);
		device_id = (0xa<<8) | (3<<3) | 0;
		enable_DMA_remapping(device_id, device_id, (uint64_t)npt);
	}
#endif
	lock_cprintf("enable_DMA_remapping return\n");
	
	iommu_inited = 1;
}

#if 0
void iommu_init(struct ivrs *ivrs)
{
	if(ivrs == NULL){
		lock_cprintf("IVRS table is not presented\n");
		return;
	}
	struct ivhd_head *ivhd = (struct ivhd_head *)((uint64_t)ivrs + 48);	
	uint32_t* capability_base;
	uint32_t base_low, base_high;
	uint64_t i,j;
	uint16_t device_id;
	extern char pgt_end[], pml4e[], npt[];
	uint64_t npt_size = (uint64_t) pgt_end - (uint64_t)pml4e;
	uint64_t npt_pml4e = (uint64_t)&npt[npt_size];
	uint64_t tmp;
	uint64_t *tt;
	int ret = 0;


	if(ivhd->type != 0x10){
		lock_cprintf("Not IVHD table\n");
		return;
	}


	memset(device_table, 0, 1024*1024*2);

	/* Set device table base addr
	 * device table base addr resigster offset is 0
	 */

	mmio_base = (uint32_t *)(ivhd->iommu_base);	
	base_low = (uint64_t)device_table & 0xfffff000;
	base_low |= 0x1ff;
	*mmio_base = base_low;
	base_high = ((uint64_t)device_table >> 32) & 0xfffff;
	*(mmio_base+1) = base_high;


	/* init intr remap table 
	 *
	 * |23--------16|15--------8| 7 | 6 | 5 |4---2| 1 | 0 |
	 * |  Vector    |   Dest    | R |DM |EOI| Type|log|EN |
	 *
	 */
	for(j=0; j<8; j++){
		for(i=0; i<256; i++){
			intr_remap_tab[j*256+i] = (i<<16) + (0x44<<8)
				+ (0<<6) + (1<<5) + ((j&1)<<2) + (0<<1) + 1;
		}
	}


	/*
	 *	|15---------8|7--------3|2-----0|
	 *	|	BUS		 |	 Device  | Func  |
	 *
	 */

	device_id = (0<<8) | (0x13<<3) | 0;	// Bus:0, Device:0x13, Function:0

	/* 
	 * Init Device 0x13, Function 0, 1, 2
	 * device table
	 */
	for(i=0; i<3; i++, device_id++){
		device_table[device_id].lint1pass = 1;
		device_table[device_id].lint0pass = 1;
		device_table[device_id].intctl = 2;
		device_table[device_id].nmipass = 1;
		device_table[device_id].einitpass = 1;
		device_table[device_id].initpass = 1;
		device_table[device_id].intr_table = (uint64_t)intr_remap_tab >>6;
		device_table[device_id].inttablen = 11;
		device_table[device_id].ig = 0;
		device_table[device_id].iv = 1;
		device_table[device_id].sysmgt = 3;
		device_table[device_id].ex = 0;
		device_table[device_id].sd = 0;
		device_table[device_id].cache = 0;
		device_table[device_id].ioctl = 1;
		device_table[device_id].sa = 0;
		device_table[device_id].se = 1;
		device_table[device_id].i = 1;
		device_table[device_id].domainid = 1;
		device_table[device_id].ir = 1;
		device_table[device_id].iw = 1;
		device_table[device_id].page_table = npt_pml4e >> 12;
		device_table[device_id].mode = 4;
		device_table[device_id].tv = 1;
		device_table[device_id].v = 1;

		enqueue_iommu_cmd(2, device_id);
		enqueue_iommu_cmd(3, device_id);
		enqueue_iommu_cmd(1, device_id);
	}

	tmp = (uint64_t)cmdbuf;
	tmp &= 0xffffffffff000;
	tmp |= 9UL<<56;
	*(mmio_base + 2) = tmp & 0xffffffff; 
	*(mmio_base + 3) = tmp >> 32;

	tmp = (uint64_t)eventbuf;
	tmp &= 0xffffffffff000;
	tmp |= 9UL<<56;
	*(mmio_base + 4) = tmp & 0xffffffff;
	*(mmio_base + 5) = tmp >> 32;

	// IOMMU Exclusion Register, Disable
	*(mmio_base + 8) = 0;	

	tmp = (1<<12) + (1<<11) + (1<<10) + (1<<9) + (1<<8) + (0<<5) + (0<<4) +
		(0<<3) + (1<<2) + (0<<1) + 1;



	//	tmp = (1<<12) + (0<<11) + (1<<10) + (0<<9) + (0<<8) + (0<<5) + (0<<4) +
	//		(0<<3) + (1<<2) + (0<<1) + 1;
	*(mmio_base + 6) = tmp;	// Enable IOMMU device 
}
#endif


uint64_t read_memory_qword(uint64_t addr)
{
	uint64_t *tmp = (uint64_t *)addr;
	return *tmp;
}

uint64_t bsf(uint64_t val)
{
	uint64_t ret;
	__asm __volatile("bsfq %%rbx, %%rax"
			:"=a"(ret)
			:"b"(val)
			:"cc");
	return ret;
}

uint64_t iopagewalk(uint64_t dte, uint64_t dva, char *guest_buffer, int *len)
{
#define LARGEST_VA(LEVEL) ((0X1000ULL << ((LEVEL)*9))-1)
#define VABITS(LEVEL) (((LEVEL)*9)+3)
#define IOPERM 0x6000000000000000UL
#define RESV_BITS 0x1ff0000000000000UL
#define U_FC_BITS 0x1800000000000000UL
#define BITS_51_12 0xffffffffff000UL


	uint64_t pdte = dte;
	uint64_t ioperm = pdte & IOPERM;
	uint64_t pa = pdte & BITS_51_12;
	uint64_t oldlevel = 7, level = (pdte>>9)&7, vabits = 63;

	if(level == 7){
		*len += sprintf(guest_buffer + *len, "DEVTAB_RESERVED_LEVEL\n");
		return 0;
	}

	if(level == 0)
		return ioperm | pa | vabits;

	while(level != 0){
		uint64_t skipbits = LARGEST_VA(oldlevel - 1) - LARGEST_VA(level);
		if((dva & skipbits) != 0){
			*len += sprintf(guest_buffer + *len, "PAGE_NOT_PRESENT 1\n");
			return 0;
		}
		uint64_t offset = (dva>>(level*9))&0xff8;
		pdte = read_memory_qword(pa + offset);
		*len += sprintf(guest_buffer + *len, "Level %d entry: %lx\n", level, pdte);
		if((pdte & 1) == 0){
			*len += sprintf(guest_buffer + *len, "PAGE_NOT_PRESENT 2\n");
			return 0;
		}

		oldlevel = level;
		level = (pdte >> 9)&7;
		uint64_t reserved_bits = RESV_BITS;
		if(level==0 || level==7){
			reserved_bits &= ~U_FC_BITS;
			ioperm |= pdte & U_FC_BITS;
		}
		if((pdte & reserved_bits) != 0){
			*len += sprintf(guest_buffer + *len, "PDTE_RESERVED_BITS 1\n");
			return 0;
		}
		ioperm &= pdte;
		pa = pdte & BITS_51_12;
		if(level == 0x7){
			uint64_t tmp = pa ^ BITS_51_12;
			vabits = bsf(tmp) + 1;
			if((vabits>=VABITS(oldlevel + 1)) ||
					(vabits<=VABITS(oldlevel))){
				*len += sprintf(guest_buffer + *len, "PDTE_RESERVED_BITS 2\n");
				return 0;
			}
			pa &= ~((1UL<<vabits)-1);
			return ioperm | pa | vabits;
		}
		if(level >= oldlevel){
			*len += sprintf(guest_buffer + *len, "PDTE_RESERVED_BITS 3\n");
			return 0;
		}
	}
	if((pa&LARGEST_VA(oldlevel-1)) != 0){
		*len += sprintf(guest_buffer + *len, "PDTE_RESERVED_BITS 4\n");
		return 0;
	}
	return ioperm | pa | VABITS(oldlevel);
}

void read_iommu_event(int iommu_id, char * guest_buffer)
{
	uint64_t head, tail, iommu_tab_b;
	uint64_t first_op, second_op, status;
	uint64_t *e, *p, *m;
	int len = 0;

	iommu_tab_b = *(mmio_base[iommu_id] + 1);
	iommu_tab_b <<= 32;
	iommu_tab_b |= *mmio_base[iommu_id];

	head = *(mmio_base[iommu_id] + 0x804);
	tail = *(mmio_base[iommu_id] + 0x806);
	head >>= 4;
	tail >>= 4;
	e = (uint64_t *)eventbuf;
	status =  *(mmio_base[iommu_id] + 0x808);
	len += sprintf(guest_buffer + len, "device_table:%p, iommu_base:%lx, status:%lx, tail:%ld\n",
			device_table, iommu_tab_b, status, tail);

	while(head != tail){
		first_op = *(e+head*2);
		second_op = *(e+head*2+1);

		m = (uint64_t *)(*(p + (second_op & ~((1UL<<39)-1))) & 0xffffffffff000UL);
		m = (uint64_t *)(m + ((second_op & 0x7fc0000000UL)>>30));
		m = (uint64_t *)&(device_table[first_op & 0xffff]);
		uint64_t tmp = 0;

		len += sprintf(guest_buffer + len, "DevID:%x, device_table base:%lx\n",
				first_op & 0xffff, &device_table[first_op]);

		len += sprintf(guest_buffer + len, "Device page table base addr: %lx\n", *m);

		tmp = iopagewalk(*m, second_op, guest_buffer, &len);

		len += sprintf(guest_buffer + len, "point:%lx, %lx, op:%lx, %lx, status:%lx, tmp:%lx\n",
				head, tail, second_op, first_op, status, tmp);
		head ++;
	}
}

