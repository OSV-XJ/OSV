#include <inc/types.h>
#include <inc/svm.h>

static unsigned long gsocket;


typedef struct 
{
	volatile unsigned int locked; /* 1: unlocked; 0: locked */
	unsigned int id;
	unsigned int pos;
}buff_des;

typedef struct
{
	unsigned char *buff;
	unsigned long length;
	unsigned int tail, head, next;
}buffer_cache;

typedef struct
{
	unsigned long sockid;
	buff_des *descr;
	buffer_cache *bcache;
}socket;

long vmm_sock(struct generl_regs *regs);
