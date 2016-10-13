#include <inc/socket.h>
#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/x86.h>
#include <inc/npt.h>
#include <inc/percpu.h>

#define ETH_DATA_LEN 1500
#define ETH_FRAME_LEN 1500
int pool_size = 2048;

struct snull_packet {
	struct snull_packet *next;
	struct net_device *dev;
	u32 datalen;
	u8 data[ETH_FRAME_LEN];
	u32 free;
//	u8 pack[18];
}__attribute__((packed));

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


struct packet_control pctl[2] __attribute__((aligned(4096), section(".data")));

struct snull_packet vmm_pkt1[2048] __attribute__((aligned(4096), section(".data")));
struct snull_packet vmm_pkt2[2048] __attribute__((aligned(4096), section(".data")));


struct packet_control *sn[2];
int snull_ready = 0;


long snull_init(struct generl_regs *regs)
{
	int id;
	uint64_t addr;
	id = read_pda(cpudid);
	if(id != 0){
		init_npt_range(node[0].base_addr + node[0].length - SNULL_RESERVE_MEM,
				SNULL_RESERVE_MEM, id, 0);
	}
	regs->rbx = node[0].base_addr + node[0].length - SNULL_RESERVE_MEM;
	lock_cprintf("Hkey in snull_init\n");
	return 0;
}

long snull_internal_init(struct generl_regs *regs)
{
	sn[0] = &pctl[0];
	sn[1] = &pctl[1];
	sn[0]->p_pkt = &vmm_pkt1[0];
	sn[1]->p_pkt = &vmm_pkt2[0];
	return 0;
}

/* packet may be overwriten*/
static struct snull_packet *snull_get_rx_pk(int id)
{

	struct snull_packet *pkt;
	
	if(sn[id]->cur_pos + 1 == sn[id]->start)
		return 0;
	pkt = &sn[id]->p_pkt[sn[id]->cur_pos];	
	return pkt;
}

long snull_tx(struct generl_regs *regs)
{
	struct snull_packet *src, *des;
	int id;

	if(!snull_ready)
		return -1;

	src = (struct snull_packet *)regs->rdi;
	id = regs->rsi;
	spin_lock((struct sem *)&sn[id]->lock);
	des = snull_get_rx_pk(id);
	if(!des)
	{
		spin_unlock((struct sem *)&sn[id]->lock);
		return -1;
	}
	memcpy(des->data, src->data, src->datalen);
	des->datalen = src->datalen;
	sn[id]->cur_pos ++;
	sn[id]->cur_pos =  sn[id]->cur_pos % pool_size;
	spin_unlock((struct sem *)&sn[id]->lock);
	return 0;
}

long snull_get(struct generl_regs *regs)
{
	int id;
	uint64_t addr;
	id = regs->rsi;
	addr = (uint64_t)sn[id];
	if(id != 0){
		init_npt_range(node[0].base_addr + node[0].length - NPT_TAB_RESERVE_MEM - SNULL_RESERVE_MEM,
				SNULL_RESERVE_MEM, id, 0);
	}
	regs->rdi = node[0].base_addr + node[0].length - NPT_TAB_RESERVE_MEM - SNULL_RESERVE_MEM;
	return 0;
}

static long vmm_sockcreate(struct generl_regs *regs)
{
	return 0;
}

static long vmm_bind(struct generl_regs *regs)
{
	return 0;
}

static long vmm_listen(struct generl_regs *regs)
{
	return 0;
}

static long vmm_accept(struct generl_regs *regs)
{
	return 0;
}

static long vmm_sendmsg(struct generl_regs *regs)
{
	return 0;
}

static long vmm_recvmsg(struct generl_regs *regs)
{
	return 0;
}

static long vmm_sockclose(struct generl_regs *regs)
{
	return 0;
}


long vmm_sock(struct generl_regs *regs)
{
	int vec = regs->rdi;
	
	switch(vec)
	{
	case 0:
		return vmm_sockcreate(regs);
	case 1:
		return vmm_bind(regs);
	case 2:
		return vmm_listen(regs);
	case 3:
		return vmm_accept(regs);
	case 4:
		return vmm_sendmsg(regs);
	case 5:
		return vmm_recvmsg(regs);
	case 6:
		return vmm_sockclose(regs);
	default:
		return -1;
	}
	return -1;
}

