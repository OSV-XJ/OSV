#ifndef ACPI_HEADER
#define ACPI_HEADER
#include <inc/types.h>

#define NR_NUMA 8

struct rsdp
{
	uint8_t sig[8];
	uint8_t check_sum;
	uint8_t oemid[6];
	uint8_t revision;
	uint32_t rsdt_addr;
	uint32_t length;
	uint64_t xsdt_addr;
	uint8_t exted_check;
	uint8_t resverd[3];
}__attribute__((packed));

struct des_header
{
	uint8_t sig[4];
	uint32_t length;
	uint8_t revision;
	uint8_t check_sum;
	uint8_t oemid[6];
	uint8_t oemtabid[8];
	uint32_t oem_revision;
	uint32_t creator_id;
	uint32_t creator_revision;
}__attribute__((packed));

struct xsdt
{
	struct des_header header;
	uint64_t entry[];
}__attribute__((packed));

struct srat
{
	struct des_header header;
	uint32_t reserved;
	uint64_t reserved1;
	uint64_t entry[];
}__attribute__((packed));

struct hpet
{
	struct des_header header;
	uint32_t block_id;
	uint8_t space_type;
	uint8_t bit_width;
	uint8_t bit_offset;
	uint8_t reserved;
	uint64_t base_address;
	uint8_t hpet_num;
	uint16_t main_counter_min;
	uint8_t	page_protection;
}__attribute__((packed));

struct madt
{
	struct des_header header;
	uint32_t apic_phy_addr;
	uint32_t flags;            //bit 0: 1 for dual 8259, 0 for no
	uint64_t entry[];
}__attribute__((packed));

struct ivrs
{
	struct des_header header;
	uint32_t ivinfo;
	uint32_t reserved1;
	uint64_t entry[];
}__attribute__((packed));

struct apic_aff
{
	uint8_t type; 					//type = 0
	uint8_t length; 				//16 bytes
	uint8_t proxi_domain;
	uint8_t apic_id;
	uint32_t flags; 			 	//bit 0 enabled/disabled, 0-31 MBZ
	uint8_t lsapic_eid;
	uint8_t proxi_domain1[3];
	uint32_t resv;
}__attribute__((packed));

struct mem_aff
{
	uint8_t type; 					//type = 1
	uint8_t length; 				//40 bytes
	uint32_t proxi_domain;
	uint16_t resv;
	uint32_t base_addr_low;
	uint32_t base_addr_high;
	uint32_t length_low;
	uint32_t length_high;
	uint32_t resv1;
	uint32_t flags; 				//bit 0 enabled/disabled, 1 hot plug, 2 non volatile,MBZ
	uint64_t resv2;
}__attribute__((packed));

struct lapic_entry
{
	uint8_t type;           //type = 0
	uint8_t length;         //8 bytes
	uint8_t acpi_processor_id;         //id for the processor listed in
												  //acpi table
	uint8_t lapicid;        //lapic id
	uint32_t flags;        //bit 0: 1 for enabled, 0 for unusable
}__attribute__((packed));

struct numa_node
{
	uint64_t id;
	uint8_t lapicid[16];
	uint64_t base_addr;
	uint64_t length;
	uint64_t index;
	uint32_t bootpid;
	uint32_t booted;
}__attribute__((packed));

extern struct numa_node node[NR_NUMA];

struct rsdp *rsdp_get(void);
struct srat *srat_get(struct xsdt *xt);
struct ivrs *ivrs_get(struct xsdt *xt);
struct hpet *hpet_get(struct xsdt *xt);
struct madt *madt_get(struct xsdt *xt);
int srat_parse(struct srat *srat);
int prepare_node_info(struct srat *srat);
int fake_acpi_lapic(struct madt *madt, int nodeid);
void *dump_madt_page(struct madt *madt, char *madt_page);
extern struct madt *madt;

#endif
