#include <inc/types.h>
#include <inc/svm.h>

typedef struct
{
	uint64_t msg_init;  /* flags for init done */
	uint64_t prev;      /*  */
	uint64_t start;     /* message tail */
	uint64_t cur_pos;   /* current position in the buff for next char */
	uint64_t cur_lenth; /* current msg length */
	uint64_t cur_start; /* latest printf start pos */
	uint64_t over_flow; /* flags for msg buffer overflow */
	uint64_t max_lenth; /* length for the buffer */
	unsigned int lock; /* sync semaphore for the vmm and os, 0:locked 1:unlocked */
	uint8_t *msg_buffer; /* the phys start address of the buffer */
	uint8_t *msg_buffer_virt; /* os virtual addr for the msg buffer */
}buff_info;

extern buff_info *info;

int vmmputchar(int ch);
int vmmcons_init(struct generl_regs *regs);
