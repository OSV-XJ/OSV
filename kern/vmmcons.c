/* say sth*/

#include <inc/types.h>
#include <inc/vmmcons.h>
#include <inc/stdio.h>
#include <inc/x86.h>
#include <inc/percpu.h>
#include <inc/osv.h>

/*
 * Using the shared memory for vmm cons output.
 * Initialization:
 * in the os, kmalloc a range of
 * space, then pass the phys addr using the vmm call.
 * Mamagement:
 * 
 */

extern struct sem *vmm_sem;

extern void *
memcpy(void *dst0, const void *src0, size_t length);

struct btstore{
	struct domain_recv_buff *recv_buff;
	char *buff_head;
	int *write_pos_back;
};


#define MAX_DOMAIN_NUM 2
//#define DOMAIN0_IP -1062731774
static struct btstore store[MAX_DOMAIN_NUM] = {{NULL, NULL, NULL}};
static char *recv_buff;
struct domain_recv_buff *domain_recv_buff;

int vmmcons_init(struct generl_regs *regs)
{
	spin_lock(&print_sem);

	if(regs->rdi < 0x1a00000)
	{
		info = 0;
		spin_unlock(&print_sem);
		return -1;
	}

	info = (buff_info *)regs->rdi;
	
	if(!info)
	{
		spin_unlock(&print_sem);
		return -1;
	}

	if(!info->msg_init)
	{
		info = 0;
		spin_unlock(&print_sem);
		return -1;
	}
				
	vmm_sem =(struct sem *) &info->lock;
	

	spin_unlock(&print_sem);
	
	return 0;
}

int vmmputchar(int ch)
{
	info->msg_buffer[info->cur_pos ++] = (uint8_t )ch;
	info->cur_lenth ++;
	if(info->cur_pos >= info->max_lenth)
	{
		info->cur_pos %= info->max_lenth;
		info->over_flow = 1;
	}
	return 0;
}

int vmgetdid(struct generl_regs *regs)
{
	
	int Did = read_pda(cpudid);

	//lock_cprintf("cpupid:%d\n",read_pda(cpupid));
	//lock_cprintf("cpudid:%d\n",read_pda(cpudid));

	regs->rbx = Did;
	return 0;
}

extern void cons_init(void);
extern void hkey_test(void);

int guest_test(struct generl_regs *regs)
{
	cons_init();

	return 0;
}


struct vmcb vmcb_save[4];
extern struct vmcb vmcb_test[];


int get_empty_entry(struct osv_table_entry *entry_tab, int table_max_num)
{
	int i, ret = -1;
	for(i=0; i<table_max_num; i++){
		if(entry_tab[i].status == OSV_STATUS_FREE){
			ret = i;
			break;
		}	
	}	
	return ret;
}

struct mm_block{
	unsigned long start;
	unsigned long len;
};

struct packet_control
{
	u32 cur_pos;
   u32 packed[15];
   u32 start;
	unsigned int lock;
	int ready;
	struct snull_packet *pkt, *p_pkt, *tpkt;
	int flag;
	int up;
}__attribute__((packed));

volatile int restart_flag = 0;
volatile int is_vmcb_saved = 0;
#define DOM_BOOT_CPU 4
extern uint64_t first[0xa0000];
extern struct packet_control *sn[2];


