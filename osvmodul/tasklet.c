/*
 * Using the tasklet to simulate the interrupt for the OSV NIC
 * Naive and simple, FIXME:!!!!!!! APIC interrupt then NAPI(POLL)
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>

struct  t_s
{
	struct tasklet_struct tlet;
	char *str;
	int i;
};


void tasklet_test(unsigned long arg)
{
	struct t_s *ts = (struct t_s *)arg;
       			
	printk(KERN_INFO"%s\n", ts->str);
	if(--ts->i)
		tasklet_schedule(&ts->tlet);
	else
	{
		printk(KERN_INFO"tasklet loop over\n");
		kfree(ts->str);
		kfree(ts);
	}
}

int __init init_tasklet(void)
{
	struct t_s *ts;

	ts = (struct t_s*)kmalloc(sizeof(ts), GFP_KERNEL);
	ts->str = (char *)kmalloc(256, GFP_KERNEL);
	sprintf(ts->str, "this is a tasklet test");
	ts->i = 1000;
	tasklet_init(&ts->tlet, tasklet_test, (unsigned long)ts);
	tasklet_schedule(&ts->tlet);
	return 0;
}

void __exit tasklet_exit(void)
{
	printk(KERN_INFO"removed\n");
}

module_init(init_tasklet);
module_exit(tasklet_exit);
