#include <inc/domain.h>
#include <inc/string.h>
#include <inc/svm.h>
#include <inc/vmpt.h>
#include <inc/cpu.h>
#include <inc/npt.h>

/*
 * Prepare for the Linux kernel and the initrd image
 */



domain_t domains[NR_DOMAINS];
//struct multiboot_info dom_mbf_info[NR_DOMAINS];

extern uint64_t Reserve_Mem;
extern uint64_t domu_save;
#if 0
static int init_e820map(int id)
{
	domain_t *domain = &domains[id];
	struct boot_params *params = domain->boot_params;
	struct e820entry *entry = params->e820_map;
	int entries = params->e820_entries;
	uint64_t start = domain->mem_start;
	uint64_t size = domain->mem_length;
	/*
	 *The e820mem map: first 640K used by all the domains.
	 *others are preserved by corresponding numa nodes.
	 */
	cprintf("start is 0x%lx\n", start);
	for(int i = 0; i < entries; i ++)
	{
		if(entry[i].type == 1)
		{
			if(entry[i].addr == 0)
			{
				//if(entry[i].size > 0x100000)
//					entry[i].size = 0x40000;
					entry[i].size = domain->mem_length;
					cprintf("entry[%d].addr == 0\n", i);
			}
			else 
			{
				entry[i].type = 2;
				continue;
				if(entry[i].addr + entry[i].size < start && entry[i].addr + entry[i].size < 0xFFFFFFFF)
				{
					entry[i].addr = VMM_RESERVED + id*0x1a000000;
					entry[i].size = 0x1a000000 - MEM_GAP;
				}
				else if(entry[i].addr + entry[i].size > start &&
						  entry[i].addr < start &&
						  entry[i].addr + entry[i].size < start + size)
				{
					entry[i].addr = start;
					entry[i].size -= (start - entry[i].addr);
					size -= entry[i].size;
					start = entry[i].addr + entry[i].size;
				}
				else if(entry[i].addr < start &&
						  entry[i].addr + entry[i].size >= start + size)
				{
					entry[i].addr = start;
					if(id == 1){
						entry[i].size = size - MEM_GAP - 1024*1024*800;
						Reserve_Mem = start + entry[i].size;
//						bkup_mem = Reserve_Mem + 1024 * 1024 * 100;
					}else
						entry[i].size = size - MEM_GAP;
				}
				else if((entry[i].addr > start && entry[i].addr +
							entry[i].size > start + size))
					entry[i].size = start + size - entry[i].addr;
				else if(entry[i].addr > start + size)
					entry[i].type = 2;
			}
		}
	}

	return 0;
}
#endif

int construct_domu_e820map(struct e820entry *original, int entries, struct e820entry *new, int did)
{
	uint64_t i, j, length = 0, offset = node[did].base_addr;
//	lock_cprintf("Domain %d e820, offset:%lx\n", did, offset);
	for(i=0,j=0; i<entries; i++){
		if(original[i].type == 1)
		{
			if(original[i].addr == 0){
				new[j].addr = 0;
				new[j].size = 0xa0000;
				new[j].type = 1;
				init_npt_range(new[j].addr, new[j].size, did, offset);
				j++;
				continue;
			}
			if(original[i].addr == 0x100000)
			{
				new[j].addr = original[i].addr;
				new[j].size = VMM_RESERVED - original[i].addr;
				new[j].type = 2;
				j++;
				new[j].addr = VMM_RESERVED;
				new[j].size = original[i].size - new[j-1].size;
				length = VMM_RESERVED + new[j].size;
//				length = new[j].size;
				new[j].type = 1;
				init_npt_range(new[j].addr, new[j].size, did, offset);
//				lock_cprintf("start:%lx, size:%lx, type:%d\n", new[j].addr, new[j].size, new[j].type);
				j++;
				continue;
			}
			if(original[i].addr == 0x100000000UL){
				new[j].addr = original[i].addr;
				new[j].size = node[did].base_addr - 0x100000000UL + length;
				new[j].type = 2;
//				lock_cprintf("start:%lx, size:%lx, type:%d\n", new[j].addr, new[j].size, new[j].type);

				j++;

				/* Reserve last 512M to store NPT and other thing */
				new[j].addr = node[did].base_addr + length;
				new[j].size = node[did].length - NPT_TAB_RESERVE_MEM - length;
				new[j].type = 1;
//				lock_cprintf("start:%lx, size:%lx, type:%d\n", new[j].addr, new[j].size, new[j].type);
				init_npt_range(new[j].addr, new[j].size, did, 0);
				j++;

				new[j].addr = new[j-1].addr + new[j-1].size;
				new[j].size = node[3].length + node[3].base_addr - new[j].addr;
				new[j].type = 2;
//				lock_cprintf("start:%lx, size:%lx, type:%d\n", new[j].addr, new[j].size, new[j].type);
				j++;
				break;
			}
		}else{
			new[j].addr = original[i].addr;
			new[j].size = original[i].size;
			new[j].type = original[i].type;
//			lock_cprintf("start:%lx, size:%lx, type:%d\n", new[j].addr, new[j].size, new[j].type);
			j++;
		}
	}
	return j;
}

int construct_e820map(struct e820entry *original, int entries, struct e820entry *new)
{
	int i, j;
	for(i=0,j=0; i<entries; i++){
		if(original[i].type == 1)
		{
			if(original[i].addr == 0){
				new[j].addr = 0;
				new[j].size = 0x9d800;
				new[j].type = 1;
				j++;
				continue;
			}
			if(original[i].addr == 0x100000)
			{
				new[j].addr = original[i].addr;
				new[j].size = VMM_RESERVED - original[i].addr;
				new[j].type = 2;
				j++;
				new[j].addr = VMM_RESERVED;
				new[j].size = original[i].size - new[j-1].size;
				new[j].type = 1;
				j++;
				continue;
			}
			if(original[i].addr>node[0].base_addr + node[0].length){
				new[j].addr = original[i].addr;
				new[j].size = original[i].size;
				new[j].type = 2;
				j++;
				continue;
			}
			if(original[i].addr + original[i].size >= node[0].base_addr + node[0].length){
				new[j].addr = original[i].addr;
				/* Reserve last 512M to store NPT and other thing */
				new[j].size = node[0].length - 512 * 1024 * 1024;
				new[j].type = 1;
				j++;
				new[j].addr = new[j-1].addr + new[j-1].size;
				new[j].size = original[i].size - new[j-1].size;
				new[j].type = 2;
				j++;
			}
		}else{
			new[j].addr = original[i].addr;
			new[j].size = original[i].size;
			new[j].type = original[i].type;
			j++;
		}
	}
	return j;
}

void create_domu(struct boot_params *params, module_t *mdl, int did)
{
	size_t size_k, size_i;
	module_t *kernel, *initrd;
	struct lheader *lh;
	char *kern_p;
	uint32_t setup_size;
	char *cmdline;
	uint64_t cmdaddr, base_addr;
	struct boot_params * boot_params;
	int cmdline_offset = 0;

	base_addr = node[did].base_addr;
	domain_t *domain = &domains[did];
	/*
	 *	Virtual address for domU is
	 *	0xa000000 * N + 0x100000
	 *
	 */
	lock_cprintf("Create DomU: %d\n", did);
	domain->k_addr = 0xa000000 * 2 + base_addr;
	domain->i_addr = 0xa000000 * 3 + base_addr;
	domain->boot_params = (struct boot_params *)(0xa000000*4 + base_addr);
	domain->dom_type = 1;	//Linux domain
	kernel = mdl;
	initrd = &mdl[1];

	cmdline = (char *)((uint64_t)kernel->string);
	cmdline_offset = 27;
	cmdline[cmdline_offset + 51] = '0' + did;
	cmdline[cmdline_offset + 66] = '1' + did;
	cmdline[cmdline_offset + 101] = '0' + did;
	lock_cprintf("cmdline:%s\n", cmdline);
	cmdaddr = (uint64_t)(domain->boot_params + 1);
	memcpy((char *)cmdaddr, cmdline, 256);



	size_k = kernel->mode_end - kernel->mode_start;
	size_i = initrd->mode_end - initrd->mode_start;
	/*kernel move*/
	memmove((void *)(uint64_t)domain->k_addr, (void *)((uint64_t)kernel->mode_start),
			size_k);
	/*initrd move*/
	memmove((void *)(uint64_t)domain->i_addr, (void *)((uint64_t)initrd->mode_start),
			size_i);
	if(size_i > 0xa000000){
		lock_cprintf("Initrd is to large\n");
		while(1);
	}

	memcpy(domain->boot_params, params, sizeof(struct boot_params));

	kern_p = (char *)(uint64_t)domain->k_addr;
	lh = get_lheader((void *)(uint64_t)domain->k_addr);
	if(lh)
	{
		lock_cprintf("Linux sig found :");
		for(uint32_t i = 0; i < 4; i ++)
			lock_cprintf("%c", lh->linux_sig[i]);
		lock_cprintf("\n");
	}
	lh->loadflags |= 0x80;
	lh->loadflags |= 0x10;
	lh->heap_end_ptr = 0x9800 - 0x200;
	lh->type_of_loader = 0x71;
	setup_size = (lh->setup_sectors + 1) * 512;
	kern_p += setup_size;

	domain->code32_start = 0xa000000;
	lh->code32_start = domain->code32_start;
	memcpy(&(domain->boot_params->hdr), &lh->setup_sectors,
			sizeof(domain->boot_params->hdr));
	memmove((void *)(uint64_t)domain->code32_start + base_addr, (void *)kern_p, size_k - setup_size);
/*	uint64_t * hkey_t = (uint64_t *)((uint64_t)domain->code32_start + base_addr);
	lock_cprintf("code32_start content:%lx, %lx, addr:%p\n",
			*hkey_t, *(hkey_t + 1), hkey_t);
*/
	domain->boot_params->hdr.cmd_line_ptr = cmdaddr - base_addr;
	domain->boot_params->hdr.ramdisk_image = domain->i_addr - base_addr;
	domain->boot_params->hdr.ramdisk_size = size_i;
	domain->boot_params->hdr.code32_start = domain->code32_start;
	domain->boot_params->screen_info.orig_video_ega_bx = 0x10;

	//	lock_cprintf("boot params initrd size is 0x%lx\n", domain->boot_params->hdr.ramdisk_size);
	//	lock_cprintf("code32_start:%x\n", domain->boot_params->hdr.code32_start);
	boot_params = domain->boot_params;
	boot_params->e820_entries = construct_domu_e820map(params->e820_map, params->e820_entries, boot_params->e820_map, did);
#if 0
	int i;
	lock_cprintf("DomainU e820 map dump\n");
	for(i=0;i<boot_params->e820_entries; i++){
		lock_cprintf("Memory start:%lx, size:%lx, type:%d\n", boot_params->e820_map[i].addr,
				boot_params->e820_map[i].size, boot_params->e820_map[i].type);
	}
	lock_cprintf("DomainU e820 map end\n");
#endif

}

void create_xen_domain(struct boot_params *params, module_t *mdl, int did)
{
	size_t size;
	module_t *xen, *kernel, *initrd;
	struct lheader *lh;
	char *kern_p;
	uint32_t setup_size, i, j, *multiboot_magic;
	char *cmdline;
	module_t *mb_mdl;
	uint64_t cmdaddr, base_addr, addr_start, addr_len;
	struct multiboot_header *m_header = NULL;
	struct multiboot_info *dom_mbf_info;
	memory_map_t *e820_mem_map;

	uint64_t *xen_code_test;

	domain_t *domain = &domains[did];

	xen = mdl;
	kernel = &mdl[1];
	initrd = &mdl[2];
	/*	lock_cprintf("xen start:%lx, end:%lx\n", xen->mode_start, xen->mode_end);
		lock_cprintf("Kernel start:%lx, end:%lx\n", kernel->mode_start, kernel->mode_end);
		lock_cprintf("initrd start:%lx, end:%lx\n", initrd->mode_start, initrd->mode_end);
		lock_cprintf("mdl addr:%lx\n", mdl);
		*/
	cmdline = (char *)((uint64_t)xen->string);
	//	lock_cprintf("Xen command line is: %s\n", cmdline);

	multiboot_magic = (uint32_t *)(uint64_t)xen->mode_start;
	for(i=0; i<8192/4;i++,multiboot_magic++){
		if(*multiboot_magic == MULTIBOOT_HEADER_MAGIC){
			m_header = (struct multiboot_header *) multiboot_magic;
			break;
		}
	}
	if(m_header == NULL){
		lock_cprintf("Cannot find Xen multiboot header\n");
		while(1);
	}
	domain->code32_start = (uint64_t)m_header - 8;
#if 0
	lock_cprintf("Find Xen multiboot header\n");
	lock_cprintf("magic: %x, flags: %x, checksum: %x\n", m_header->magic, m_header->flags, m_header->checksum);
	lock_cprintf("header_addr: %x, load_addr: %x, load_end_addr: %x\n",
			m_header->header_addr, m_header->load_addr, m_header->load_end_addr);
	lock_cprintf("bss_end_addr: %x, entry_addr: %x, mode_type: %x\n",
			m_header->bss_end_addr, m_header->entry_addr, m_header->mode_type);
	lock_cprintf("width: %x, height: %x, depth: %x\n",
			m_header->width, m_header->height, m_header->depth);
	getchar();
#endif


	/* including command line, modules, and mmap info

		|***********|**************|*****|****************|**************|*********|**************|********|********|
		| CMOS data | VMM RESERVED | xen | multiboot_info | command line | e820map | module count | kernel | module |
		0       0xa0000       0x3c00000         sizeof()        +256      +sizeof() 

*/

	size = xen->mode_end - domain->code32_start;
	base_addr = VMM_RESERVED;
	memmove((void *)base_addr, (void *)(uint64_t)domain->code32_start, size);
	domain->code32_start = 0x100000;

	dom_mbf_info = (struct multiboot_info *) ((VMM_RESERVED + size + 0x1000) & ~0xfffUL);
	domain->mb_info_addr = (uint64_t) dom_mbf_info - VMM_RESERVED + 0x100000;
	//	lock_cprintf("multiboot information phy addr:%lx, logical addr:%lx\n",
	//			dom_mbf_info, domain->mb_info_addr);

	dom_mbf_info->flags = (1<<0) | (1<<2) | (1<<3) | (1<<6) | (1<<9);
	dom_mbf_info->cmdline = (uint64_t)(dom_mbf_info + 1);
	dom_mbf_info->mem_lower = 0x280;
	dom_mbf_info->mem_upper = 0x2eede4;
	memcpy((char *)(uint64_t)dom_mbf_info->cmdline, (void *)cmdline, 256);
	memcpy((char *)(uint64_t)dom_mbf_info->cmdline+240, "OSV\0", 4);
	dom_mbf_info->mmap_addr = dom_mbf_info->cmdline + 256;	// cmdline 240 bytes, 16 gap
	e820_mem_map =(memory_map_t *)(uint64_t)dom_mbf_info->mmap_addr;
	dom_mbf_info->cmdline -= VMM_RESERVED - 0x100000;
	dom_mbf_info->boot_loader_name = dom_mbf_info->cmdline + 240;
	//	lock_cprintf("Multiboot cmdline addr:%lx\n", dom_mbf_info->cmdline);
	for(i=0,j=0; i<params->e820_entries; i++){
		if(params->e820_map[i].addr >
				node[0].base_addr + node[0].length){
			break;
		}
		if(params->e820_map[i].addr == 0x100000){
			e820_mem_map[j].base_addr_low = 0x100000;
			e820_mem_map[j].base_addr_high = 0x0;
			e820_mem_map[j].length_low = 0x40000000 - VMM_RESERVED;
			e820_mem_map[j].length_high = 0x0;
			e820_mem_map[j].type = 1;
			e820_mem_map[j].size = 20;
			init_npt_range(0x100000, 0x40000000 - VMM_RESERVED,
					did, VMM_RESERVED - 0x100000);
			j++;
			e820_mem_map[j].base_addr_low = 0x40000000 - VMM_RESERVED + 0x100000;
			e820_mem_map[j].base_addr_high = 0x0;
			e820_mem_map[j].length_low = VMM_RESERVED - 0x100000;
			e820_mem_map[j].length_high = 0x0;
			e820_mem_map[j].type = 2;
			e820_mem_map[j].size = 20;
			j++;

			e820_mem_map[j].base_addr_low = 0x40000000;
			e820_mem_map[j].base_addr_high = 0x0;
			e820_mem_map[j].length_low = params->e820_map[i].size + 0x100000 - 0x40000000;
			e820_mem_map[j].length_high = 0x0;
			e820_mem_map[j].type = 1;
			e820_mem_map[j].size = 20;
			lock_cprintf("start:%lx, end:%lx\n", e820_mem_map[j].base_addr_low +
					((uint64_t)e820_mem_map[j].base_addr_high << 32),
					e820_mem_map[j].base_addr_low +
					((uint64_t)e820_mem_map[j].base_addr_high << 32) +
					e820_mem_map[j].length_low + 
					((uint64_t)e820_mem_map[j].length_high << 32)); 
			init_npt_range(0x40000000, e820_mem_map[j].length_low ,
					did, 0);
			j++;

			continue;
		}
		e820_mem_map[j].base_addr_low = params->e820_map[i].addr & 0xffffffff;
		e820_mem_map[j].base_addr_high = params->e820_map[i].addr >> 32;
		e820_mem_map[j].length_low = params->e820_map[i].size & 0xffffffff;
		e820_mem_map[j].length_high = params->e820_map[i].size >> 32;
		e820_mem_map[j].type = params->e820_map[i].type;
		e820_mem_map[j].size = 20;

		addr_start = params->e820_map[i].addr;
		addr_len = params->e820_map[i].size;

		if(addr_start + addr_len >=
				node[0].base_addr + node[0].length){
			e820_mem_map[j].length_low = (node[0].base_addr+node[0].length - 
					addr_start - NPT_TAB_RESERVE_MEM) & 0xffffffff;
			e820_mem_map[j].length_high = (node[0].base_addr+node[0].length - 
					addr_start - NPT_TAB_RESERVE_MEM) >> 32;
			init_npt_range(addr_start, node[0].base_addr+node[0].length - addr_start - NPT_TAB_RESERVE_MEM, did, 0);
			lock_cprintf("start:%lx, end:%lx\n", e820_mem_map[j].base_addr_low +
					((uint64_t)e820_mem_map[j].base_addr_high << 32),
					e820_mem_map[j].base_addr_low +
					((uint64_t)e820_mem_map[j].base_addr_high << 32) +
					e820_mem_map[j].length_low + 
					((uint64_t)e820_mem_map[j].length_high << 32)); 
			i++;
			j++;
			break;
		}
		lock_cprintf("start:%lx, end:%lx\n", e820_mem_map[j].base_addr_low +
				((uint64_t)e820_mem_map[j].base_addr_high << 32),
				e820_mem_map[j].base_addr_low +
				((uint64_t)e820_mem_map[j].base_addr_high << 32) +
				e820_mem_map[j].length_low + 
				((uint64_t)e820_mem_map[j].length_high << 32));

		if(params->e820_map[i].type == 1){
			init_npt_range(addr_start, addr_len, did, 0);
		}
		j++;
	}
	lock_cprintf("node[0] start:%lx, length:%lx\n", node[0].base_addr, node[0].length);
	//	getchar();
	dom_mbf_info->mmap_length = j*sizeof(memory_map_t);
	dom_mbf_info->mods_addr = dom_mbf_info->mmap_length + dom_mbf_info->mmap_addr;
	dom_mbf_info->mods_count = 2;
	mb_mdl = (module_t *)(uint64_t)dom_mbf_info->mods_addr;
	dom_mbf_info->mmap_addr -= VMM_RESERVED - 0x100000;


	base_addr = (dom_mbf_info->mods_addr + 2*sizeof(module_t) + 0x1000) & ~0xfffUL;
	dom_mbf_info->mods_addr -= VMM_RESERVED - 0x100000;
	size = kernel->mode_end - kernel->mode_start;
	memmove((void *)base_addr, (void *)((uint64_t)kernel->mode_start), size);
	mb_mdl[0].mode_start = base_addr - VMM_RESERVED + 0x100000;
	if(kernel->string){
		mb_mdl[0].string = base_addr + size;
		memcpy((char *)(uint64_t)mb_mdl[0].string, (void *)(uint64_t)kernel->string, 256);
		mb_mdl[0].string -= VMM_RESERVED - 0x100000;
	}else{
		mb_mdl[0].string = 0;
	}
	mb_mdl[0].mode_end = mb_mdl[0].mode_start + size;

	base_addr += size + 256;
	base_addr = (base_addr + 0x1000) & ~0xfffUL;
	size = initrd->mode_end - initrd->mode_start;
	memmove((void *)base_addr, (void *)((uint64_t)initrd->mode_start), size);
	mb_mdl[1].mode_start = base_addr - VMM_RESERVED + 0x100000;
	if(initrd->string){
		mb_mdl[1].string = base_addr + size;
		memcpy((void *)(uint64_t)mb_mdl[1].string, (void *)(uint64_t)initrd->string, 256);
		mb_mdl[1].string -= VMM_RESERVED - 0x100000;
	}else{
		mb_mdl[1].string = 0;
	}
	mb_mdl[1].mode_end = mb_mdl[1].mode_start + size;
	//	lock_cprintf("module 0 start:%lx, end:%lx\n", mb_mdl[0].mode_start, mb_mdl[0].mode_end);
	//	lock_cprintf("module 1 start:%lx, end:%lx, size:%lx, first value:%lx\n", mb_mdl[1].mode_start, mb_mdl[1].mode_end, size, *(uint64_t *)base_addr);
	//	getchar();

}

void create_linux_domain(struct boot_params *params, module_t *mdl, int did)
{
	size_t size_k, size_i;
	module_t *kernel, *initrd;
	struct lheader *lh;
	char *kern_p;
	uint32_t setup_size;
	char *cmdline;
	uint64_t cmdaddr;
	struct boot_params * boot_params;
	int i, j;

	domain_t *domain = &domains[did];
	domain->k_addr = VMM_RESERVED + 0xa000000 * 2;
	domain->i_addr = VMM_RESERVED + 0xa000000 * 3;
	domain->boot_params = (struct boot_params *)0xa000000;
	kernel = mdl;
	initrd = &mdl[1];

	cmdline = (char *)((uint64_t)kernel->string);

	cmdaddr = (uint64_t)(domain->boot_params + 1);

	memcpy((char *)cmdaddr, cmdline, 256);


	size_k = kernel->mode_end - kernel->mode_start;
	size_i = initrd->mode_end - initrd->mode_start;

	/*	lock_cprintf("kernel start 0x%lx, end at 0x%lx\n", kernel->mode_start, kernel->mode_end);
		lock_cprintf("initrd start 0x%lx, end at 0x%lx\n", initrd->mode_start, initrd->mode_end);
		lock_cprintf("kernel start 0x%lx\n", domain->k_addr);
		lock_cprintf("initrd start 0x%lx\n", domain->i_addr);
		*/

	/*kernel move*/
	memmove((void *)(uint64_t)domain->k_addr, (void *)((uint64_t)kernel->mode_start),
			size_k);

	/*initrd move*/
	memmove((void *)(uint64_t)domain->i_addr, (void *)((uint64_t)initrd->mode_start),
			size_i);

	memcpy(domain->boot_params, params, sizeof(struct boot_params));

	//	lock_cprintf("here at create domain %lx\n", cmdaddr);
	kern_p = (char *)(uint64_t)domain->k_addr;
	lh = get_lheader((void *)(uint64_t)domain->k_addr);
	if(lh)
	{
		lock_cprintf("Linux sig found :");
		for(uint32_t i = 0; i < 4; i ++)
			lock_cprintf("%c", lh->linux_sig[i]);
		lock_cprintf("\n");
	}
	lh->loadflags |= 0x80;
	lh->loadflags |= 0x10;
	lh->heap_end_ptr = 0x9800 - 0x200;
	lh->type_of_loader = 0x71;
	setup_size = (lh->setup_sectors + 1) * 512;
	kern_p += setup_size;

	domain->code32_start = 0xa000000 + VMM_RESERVED;
	lh->code32_start = domain->code32_start;;
	memcpy(&(domain->boot_params->hdr), &lh->setup_sectors,
			sizeof(domain->boot_params->hdr));
	memmove((void *)(uint64_t)domain->code32_start, (void *)kern_p, size_k - setup_size);

	domain->boot_params->hdr.cmd_line_ptr = cmdaddr;
	domain->boot_params->hdr.ramdisk_image = domain->i_addr;
	domain->boot_params->hdr.ramdisk_size = size_i;
	domain->boot_params->hdr.code32_start = domain->code32_start;
	domain->boot_params->screen_info.orig_video_ega_bx = 0x10;

	//	lock_cprintf("boot params initrd size is 0x%lx\n", domain->boot_params->hdr.ramdisk_size);
	//	lock_cprintf("code32_start:%x\n", domain->boot_params->hdr.code32_start);
	boot_params = domain->boot_params;
	for(i=0, j=0; i<params->e820_entries; i++){
		if(params->e820_map[i].addr >
				node[did].base_addr + node[did].length){
			break;
		}
		if(params->e820_map[i].addr == 0x100000){
			boot_params->e820_map[j].addr = 0x100000;
			boot_params->e820_map[j].size = VMM_RESERVED - params->e820_map[i].addr;
			boot_params->e820_map[j].type = 2;
			j++;
			boot_params->e820_map[j].addr = VMM_RESERVED;
			boot_params->e820_map[j].size = params->e820_map[i].size - boot_params->e820_map[j-1].size;
			boot_params->e820_map[j].type = 1;
			init_npt_range(boot_params->e820_map[j].addr, boot_params->e820_map[j].size,
					did, 0);
//			lock_cprintf("Start:%lx, End:%lx\n", boot_params->e820_map[j].addr,
//					boot_params->e820_map[j].addr + boot_params->e820_map[j].size);

			j++;
			continue;
		}
		boot_params->e820_map[j].addr = params->e820_map[i].addr;
		boot_params->e820_map[j].size = params->e820_map[i].size;
		boot_params->e820_map[j].type = params->e820_map[i].type;

		if(boot_params->e820_map[j].addr + boot_params->e820_map[j].size >=
				node[0].base_addr + node[0].length){
			boot_params->e820_map[j].size = node[0].base_addr+node[0].length - 
				boot_params->e820_map[j].addr - NPT_TAB_RESERVE_MEM - SNULL_RESERVE_MEM;
			init_npt_range(boot_params->e820_map[j].addr, boot_params->e820_map[j].size + SNULL_RESERVE_MEM,
					did, 0);
//			lock_cprintf("Start:%lx, End:%lx\n", boot_params->e820_map[j].addr,
//					boot_params->e820_map[j].addr + boot_params->e820_map[j].size);

			i++;
			j++;
			break;
		}
		if(boot_params->e820_map[j].type == 1){
//			lock_cprintf("Start:%lx, End:%lx\n", boot_params->e820_map[j].addr,
//					boot_params->e820_map[j].addr + boot_params->e820_map[j].size);
			init_npt_range(boot_params->e820_map[j].addr, boot_params->e820_map[j].size,
					did, 0);
		}
		j++;
	}
	boot_params->e820_entries = j;
#if 0
	for(i=0;i<boot_params->e820_entries; i++){
		lock_cprintf("Memory start:%lx, size:%lx, type:%d\n", boot_params->e820_map[i].addr,
				boot_params->e820_map[i].size, boot_params->e820_map[i].type);
	}
#endif
	//	boot_params->e820_entries = construct_e820map(params->e820_map, params->e820_entries, boot_params->e820_map);
}

int create_domain(struct boot_params *params, module_t *mdl, int did)
{
	struct lheader *lh;

	lh = get_lheader((void *)(uint64_t)mdl->mode_start);
	if(lh)
	{
		/*	lock_cprintf("Linux sig found :");
			for(uint32_t i = 0; i < 4; i ++)
			lock_cprintf("%c", lh->linux_sig[i]);
			lock_cprintf("  %lx\n", lh);
			*/
		create_linux_domain(params, mdl, did);
		domains[did].dom_type = 1; // This is a Linux domain
	}else{
		//	lock_cprintf("This is a Xen kernel, %lx\n", lh);
		create_xen_domain(params, mdl, did);
		domains[did].dom_type = 2; // This is a Xen domain
	}
	return 0;
}

#if 0
int init_maindomain(struct boot_params *param, uint64_t kaddr)
{
	domain_t *domain = &domains[0];
	domain->boot_params = param;
	domain->code32_start = kaddr;
	return 0;
}
#endif

void start_vm(struct generl_regs *regs)
{
	int id = regs->rdi;
	extern int npt_init;
	npt_init = 0;

	lock_cprintf("Start Domain %ld\n", regs->rdi);

	create_mpt(gfptr);
	mpt_domain_set(id, (void *)VMPT_ADDR);
	cpus[lapicid_to_index[node[id].bootpid]].svm = 1;
}
