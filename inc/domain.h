#include <inc/types.h>
#include <inc/stdio.h>
#include <inc/linux-header.h>
#include <kern/multiboot1.h>
#include <boot/bootparam.h>
#include <inc/acpi.h>

typedef struct
{
	uint64_t k_addr;
	uint64_t i_addr;
	uint64_t mb_info_addr;
	uint32_t mem32_start;
	uint32_t mem32_length;
	uint32_t code32_start;
	uint32_t dom_type;
	struct boot_params *boot_params;
	uint64_t mem_start, mem_length;       /* continous memory node */
}domain_t;

#define NR_DOMAINS 8
#define VMM_RESERVED 0x3d00000
#define MEM_GAP 0x100000
//#define NPT_START (1<<30)
//#define NPT_SIZE	(1<<30)

extern domain_t domains[NR_DOMAINS];
extern struct boot_params *boot_params;


int init_domain(int node);
int create_domain(struct boot_params *params, module_t *mdl, int did);
void create_domu(struct boot_params *params, module_t *mdl, int did);
int init_maindomain(struct boot_params *params, uint64_t kaddr);
void reboot_domain(void);

static void dump_domain(int id)
{
	struct e820entry *entry;
	char *sig;
	int entries;

	entry = domains[id].boot_params->e820_map;
	entries = domains[id].boot_params->e820_entries;
	sig =(char *)( &domains[id].boot_params->hdr.header);
	
//	lock_cprintf("domain %d\n e820 map start , size  ,  type\n", id);
	
/*	for(int i = 0; i < entries; i ++)
		lock_cprintf("0x%lx, 0x%lx, %lx\n", entry[i].addr,
						 entry[i].size, entry[i].type);*/
	for(int j = 0; j < 4; j ++)
		lock_cprintf("%c", sig[j]);
	lock_cprintf("\n");
	lock_cprintf("in dump_domain \n");
}
