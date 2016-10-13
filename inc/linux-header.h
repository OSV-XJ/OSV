#include <inc/types.h>
#include <inc/linuxdef.h>


struct lheader
{
	uint8_t code[0x1f0];
	uint8_t pad;
	uint8_t setup_sectors;
	uint16_t root_flags;
	uint32_t syssize;
	uint16_t ram_size;
	uint16_t vid_mode;
	uint16_t root_dev;
	uint16_t boot_flags; /*AA55*/

	uint16_t jmp_instr;
	uint8_t linux_sig[4]; /*"HdrS"*/
	uint16_t version;
	uint32_t realmode_switch;
	uint32_t start_sys_seg;
	uint8_t type_of_loader;
	uint8_t loadflags;
	uint16_t setup_move_size;
	uint32_t code32_start;
	uint32_t ramdisk_addr;
	uint32_t ramdisk_size;
	uint32_t bootsect_kludge;
	uint16_t heap_end_ptr;
	uint16_t pad1;
	uint32_t cmd_line_ptr;
	uint32_t ramdisk_max;
}__attribute__((__packed__));

struct lheader *get_lheader(void *addr);

void linux_move(void *addr, uint32_t size);
void readseg(uint64_t va, uint32_t count, uint32_t offset);
