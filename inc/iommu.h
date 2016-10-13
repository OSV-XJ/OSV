#ifndef IOMMU_HEADER
#define IOMMU_HEADER

#include <inc/acpi.h>
#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/privacy.h>

struct ivhd_head{
	uint8_t type;
	uint8_t flags;
	uint16_t length;
	uint16_t dev_id;
	uint16_t cap_offset;
	uint64_t iommu_base;
	uint16_t pci_group;
	uint16_t iommu_info;
	uint32_t reserved;
//	uint64_t dev_entries[];
}__attribute__((__packed__));

struct ivmd_head{
	uint8_t type;
	uint8_t flags;
	uint16_t length;
	uint16_t device_id;
	uint16_t auxiliary_data;
	uint64_t reserved;
	uint64_t ivmd_start_addr;
	uint64_t ivmd_mm_blk_length;
}__attribute__((__packed__));


struct iommu_command{
	uint32_t first_op_low;
	uint32_t first_op_high;
	uint32_t second_op_low;
	uint32_t second_op_high;
};

struct dev_table{
	uint64_t v:1;
	uint64_t tv:1;
	uint64_t reserved:7;
	uint64_t mode:3;
	uint64_t page_table:40;
	uint64_t reserved1:9;
	uint64_t ir:1;
	uint64_t iw:1;
	uint64_t reserved2:1;
	uint16_t domainid;
	uint16_t reserved3;
	uint32_t i:1;
	uint32_t se:1;
	uint32_t sa:1;
	uint32_t ioctl:2;
	uint32_t cache:1;
	uint32_t sd:1;
	uint32_t ex:1;
	uint32_t sysmgt:2;
	uint32_t reserved4:22;
	uint64_t iv:1;
	uint64_t inttablen:4;
	uint64_t ig:1;
	uint64_t intr_table:46;
	uint64_t reserved5:4;
	uint64_t initpass:1;
	uint64_t einitpass:1;
	uint64_t nmipass:1;
	uint64_t reserved6:1;
	uint64_t intctl:2;
	uint64_t lint0pass:1;
	uint64_t lint1pass:1;
	uint64_t reserved7;
}__attribute__((__packed__));


void iommu_init(struct ivrs *ivrs);
void read_iommu_event(int iommu_id, char *guest_buffer);

#define IOMMU_BUF_SIZE (4096*2)
#define MAX_IOMMU 2
#define MAX_DEV_ID 0x3000


extern struct dev_table device_table[MAX_DEV_ID];

#endif
