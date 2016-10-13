#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/smp.h>
#include <linux/spinlock.h>

#include <asm/uaccess.h>
#include <asm/io.h>

#include "osvstd.h"


MODULE_LICENSE("GPL");


static struct proc_dir_entry *vmm_cons;



#define VMMMSG_BASE 0xB8000

#define MSG_BUFF_SIZE 0x1000

typedef struct
{
	u64 msg_init;
	u64 prev;
	u64 start;
	u64 cur_pos;
	u64 cur_lenth;
	u64 cur_start;
	u64 over_flow;
	u64 max_lenth;
	unsigned int  lock;
	u8 *msg_buffer;
	u8 *msg_buffer_virt;
}buff_info;


buff_info *info;

int vmm_msg_init(void)
{

	info =(buff_info *) kmalloc(sizeof(buff_info), GFP_KERNEL);
	

	/* the space returned by kmalloc is consecutive in phys */
	info->msg_buffer_virt = kmalloc(MSG_BUFF_SIZE, GFP_KERNEL);
	if(!info->msg_buffer_virt)
		return -1;
	
	info->msg_buffer = (u8 *)virt_to_phys((void *)info->msg_buffer_virt);
	info->msg_init = 1;
	info->cur_pos = 0;
	info->start = 0;
	info->prev = 0;
	info->cur_lenth = 0;
	info->over_flow = 0;
	info->lock = 1;
	info->max_lenth = MSG_BUFF_SIZE;
	
	
	vmmcons(virt_to_phys((void *)info));
	
	return 0;
}


size_t get_vmm_msg(void *buffer, size_t size, void *sig)
{
	return 0;
}

int vmmcons_read(char *buffer, char **start, off_t off,
					  int count, int *eof, void *data)
{
	int size;
	int i;
	
	__raw_spin_lock((raw_spinlock_t *) &info->lock);
	
	*eof = 1;
	size = info->cur_pos >= info->start? (info->cur_pos - info->start)
		:(info->cur_pos + info->max_lenth - info->start);
	
	for(i = 0; i < size; i ++)
		buffer[i] = info->msg_buffer_virt[(info->start + i) % info->max_lenth];

	printk(KERN_INFO"the vmmcons size is %d\n", size);
	
	__raw_spin_unlock((raw_spinlock_t *) &info->lock);
	
	
	return size;
}

static int __init vmm_msg_mod_init(void)
{
	if(vmm_msg_init())
		return -1;

	vmm_cons = create_proc_read_entry("vmmcons", 0644, NULL,
												vmmcons_read, NULL);

	return 0;
}


static void __exit vmm_msg_mod_exit(void)
{

	remove_proc_entry("vmmcons", NULL);

	vmmcons(0);
	
	info->msg_init = 0;
	info->cur_pos = 0;
	info->over_flow = 0;
	
	kfree(info->msg_buffer_virt);
	kfree(info);

	return;
}

void sendmsg(void *msg, size_t size)
{
	
}

module_init(vmm_msg_mod_init);
module_exit(vmm_msg_mod_exit);

