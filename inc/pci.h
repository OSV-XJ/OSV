#ifndef PCI_HEADER
#define PCI_HEADER

#include <inc/types.h>
#include <inc/x86.h>
#include <inc/vmcb.h>
#include <inc/percpu.h>
#include <inc/stdio.h>

#define USB_KBD_PCI_ADDR (0x13<<11)
struct pci_config_header{
	uint16_t vendor_id;
	uint16_t device_id;
	uint16_t command;
	uint16_t status;
	uint8_t revision_id;	//0x08
	uint8_t class_code0;
	uint8_t class_code1;
	uint8_t class_code2;
	uint8_t cache_line_size;
	uint8_t latency;
	uint8_t type;
	uint8_t bist;
	uint64_t base_addr0;	//0x10
	uint64_t base_addr1;
	uint64_t base_addr2;
	uint32_t cis_pointer;	//0x28
	uint16_t sub_vendor_id;
	uint16_t subsystem_id;
	uint32_t ex_rom_base;
	uint8_t capabilities;
	uint8_t reserved[7];
	uint8_t intr_line;//0x3c
	uint8_t intr_pin;
	uint8_t min_gnt;
	uint8_t max_lat;
}__attribute__((__packed__));

struct pci_bridge_header{
	uint16_t vendor_id;
	uint16_t device_id;
	uint16_t command;
	uint16_t status;
	uint8_t revision_id;
	uint8_t class_code0;
	uint8_t class_code1;
	uint8_t class_code2;
	uint8_t cache_line_size;
	uint8_t primary_latency;
	uint8_t type;
	uint8_t bist;
	uint32_t base_addr0;	//0x10
	uint32_t base_addr1;
	uint8_t primary_bus;
	uint8_t second_bus;
	uint8_t sub_bus;
	uint8_t second_latency;
	uint8_t io_base;
	uint8_t io_limit;
	uint16_t second_status;
	uint16_t mem_base;
	uint16_t mem_limit;
	uint16_t prefet_mem_base;
	uint16_t prefet_mem_limit;
	uint32_t prefet_base_upper;
	uint32_t prefet_limit_upper;
	uint16_t io_base_upper;
	uint16_t io_limit_upper;
	uint8_t capabilities;
	uint8_t reserved[3];
	uint32_t ex_rom_base;
	uint8_t intr_line;//0x3c
	uint8_t intr_pin;
	uint16_t bridge_control;
}__attribute__((__packed__));

void pci_config_access(struct vmcb *vmcb);



int get_pci_header(uint32_t dev_id, void *addr);
void get_pci_dw(uint32_t dev_id, int offset, uint32_t *val);


#endif
