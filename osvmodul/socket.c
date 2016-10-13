#include <linux/kernel.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");


int socket()
{
	char *buff = kmalloc(4096*2, GFP_KERNEL);
	int id;
	
	id = vmmsock();
	
	return 0;
}

int send()
{
	return 0;
}

int recieve()
{
	return 0;
}

int accept()
{
	return 0;
}

int bind()
{
	return 0;
}

int listen()
{
	return 0;
}
