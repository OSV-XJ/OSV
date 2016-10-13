/*
 * snull.c --  the Simple Network Utility
 *
 * Copyright (C) 2001 Alessandro Rubini and Jonathan Corbet
 * Copyright (C) 2001 O'Reilly & Associates
 *
 * The source code in this file can be freely used, adapted,
 * and redistributed in source or binary form, so long as an
 * acknowledgment appears in derived source files.  The citation
 * should list that the code comes from the book "Linux Device
 * Drivers" by Alessandro Rubini and Jonathan Corbet, published
 * by O'Reilly & Associates.   No warranty is attached;
 * we cannot take responsibility for errors or fitness for use.
 *
 * $Id: snull.c,v 1.21 2004/11/05 02:36:03 rubini Exp $
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/moduleparam.h>

#include <linux/sched.h>
#include <linux/kernel.h> /* printk() */
#include <linux/slab.h> /* kmalloc() */
#include <linux/errno.h>  /* error codes */
#include <linux/types.h>  /* size_t */
#include <linux/interrupt.h> /* mark_bh */
#include <linux/percpu.h>
#include <asm/io.h>

#include <linux/in.h>
#include <linux/netdevice.h>   /* struct device, and other headers */
#include <linux/etherdevice.h> /* eth_type_trans */
#include <linux/ip.h>          /* struct iphdr */
#include <linux/tcp.h>         /* struct tcphdr */
#include <linux/skbuff.h>

#include <linux/in6.h>
#include <asm/checksum.h>

#include "snull.h"
#include "osvstd.h"

#define MAX_DOMAINS 2

MODULE_AUTHOR("Alessandro Rubini, Jonathan Corbet");
MODULE_LICENSE("Dual BSD/GPL");


int id = 0;

/*
 * A structure representing an in-flight packet.
 */
struct snull_packet {
	struct snull_packet *next;
	struct net_device *dev;
	u32 datalen;
	u8 data[ETH_DATA_LEN];
	u32 free;
}__attribute__((packed));


struct snull_packet *tx_snull_pkt[MAX_DOMAINS];

int pool_size = 2048;

/*
 * This structure is private to each device. It is used to pass
 * packets in and out, so there is place for a packet
 */

struct snull_priv {
	struct net_device_stats stats;
	void *lstats;
	int status;
	struct snull_packet *ppool;
	struct snull_packet *rx_queue;  /* List of incoming packets */
	int rx_int_enabled;
	int tx_packetlen;
	u8 *tx_packetdata;
	struct sk_buff *skb;
	spinlock_t lock;
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
}__attribute__((packed));

struct pcpu_lstats
{
	unsigned long tpackets;
	unsigned long rpackets;
	unsigned long tbytes;
	unsigned long rbytes;
};


struct packet_control *sn[MAX_DOMAINS];

static void snull_tx_timeout(struct net_device *dev);

struct snull_tlet_arg
{
	struct tasklet_struct tlet;
	struct net_device *dev;
};
struct snull_tlet_arg *stl;

#define IRQ_NUM 9 
irqreturn_t ipi_handler(int vec, void *dev_id)
{
	tasklet_schedule(&stl->tlet);
	ack_APIC_irq();
//	printk(KERN_INFO "Recieve a IPI --> vec: %d\n", vec);
	return IRQ_HANDLED;
}

void send_dataready_ipi(int cpu_phy_id)
{
	if(cpu_phy_id == 0)
		cpu_phy_id = 0x20;
	else
		cpu_phy_id = 0x44;
	native_apic_mem_write(0x310, cpu_phy_id<<24);
	native_apic_mem_write(0x300, 0x30 + IRQ_NUM);
//	printk(KERN_INFO "Send a IPI\n");
}

void snull_teardown_pool(struct net_device *dev)
{
	kfree(sn[id]->pkt);
	kfree(sn[id]);
}    
    
/*
 * Open and close
 */

int snull_open(struct net_device *dev)
{
	/* request_region(), request_irq(), ....  (like fops->open) */

	/* 
	 * Assign the hardware address of the board: use "\0SNULx", where
	 * x is 0 or 1. The first byte is '\0' to avoid being a multicast
	 * address (the first byte of multicast addrs is odd).
	 */
	memcpy(dev->dev_addr, "\0SNUL0", ETH_ALEN);
	dev->dev_addr[ETH_ALEN - 1] += id;
	netif_start_queue(dev);
	return 0;
}

static void snull_dev_free(struct net_device *dev)
{

}

int snull_release(struct net_device *dev)
{
    /* release ports, irq and such -- like fops->close */

	netif_stop_queue(dev); /* can't transmit any more */
	return 0;
}

/*
 * Configuration changes (passed on by ifconfig)
 */
int snull_config(struct net_device *dev, struct ifmap *map)
{
	if (dev->flags & IFF_UP) /* can't act on a running interface */
		return -EBUSY;

	/* Don't allow changing the I/O address */
	if (map->base_addr != dev->base_addr) {
		printk(KERN_WARNING "snull: Can't change I/O address\n");
		return -EOPNOTSUPP;
	}

	/* Allow changing the IRQ */
	if (map->irq != dev->irq) {
		dev->irq = map->irq;
        	/* request_irq() is delayed to open-time */
	}

	/* ignore other fields */
	return 0;
}


/*struct snull_tlet_arg
{
	struct tasklet_struct tlet;
	struct net_device *dev;
};*/


/*
 * Receive a packet: retrieve, encapsulate and pass over to upper levels
 */
void snull_rx(struct net_device *dev, struct snull_packet *pkt)
{
	struct sk_buff *skb;
	struct snull_priv *priv = netdev_priv(dev);
	struct pcpu_lstats *pcpu_lstats, *lb_stats;

	char *buf;
	unsigned char* saddr;
	struct iphdr* ih;

	/*
	 * The packet has been retrieved from the transmission
	 * medium. Build an skb around it, so upper layers can handle it
	 */
	//printk(KERN_INFO "Recieve a packet\n");
	skb = dev_alloc_skb(pkt->datalen + 2);
	//printk(KERN_INFO"sn0 rx packet size is 0x%x\n", pkt->datalen);
	if (!skb) {
		if (printk_ratelimit())
			printk(KERN_NOTICE "snull rx: low on mem - packet dropped\n");
		priv->stats.rx_dropped++;
		goto out;
	}
	skb_reserve(skb, 2); /* align IP on 16B boundary */  
	memcpy(skb_put(skb, pkt->datalen), pkt->data, pkt->datalen);

	/* Write metadata, and then pass to the receive level */
	skb->dev = dev;
	skb->protocol = eth_type_trans(skb, dev);
	skb->ip_summed = CHECKSUM_UNNECESSARY; /* don't check it */

	buf = pkt->data;
	ih = (struct iphdr*)(buf + sizeof(struct ethhdr));
	saddr = (unsigned char*)&ih->saddr;
	//printk(KERN_INFO "Dom0 recieve: %d.%d.%d.%d\n", saddr[0], saddr[1], saddr[2], saddr[3]);

	pcpu_lstats = priv->lstats;
	lb_stats = per_cpu_ptr(pcpu_lstats, smp_processor_id());
	lb_stats->rbytes += pkt->datalen;
	lb_stats->rpackets ++;

	if(sn[id]->start == pool_size - 1)
		sn[id]->start = 0;
	else
		sn[id]->start ++;
	netif_rx(skb);
  out:
	return;
}

int snull_removed = 0;


void snull_tasklet_rx(unsigned long arg)
{
	struct snull_tlet_arg *starg = (struct snull_tlet_arg *)arg;
	struct net_device *dev;
	struct snull_priv *priv; 

	dev = starg->dev;
	priv = netdev_priv(dev);
	
	//printk(KERN_INFO "Dom0: has reached snull_tasklet_rx\n");
/*	spin_lock(&priv->lock);
	num = sn[id]->cur_pos >= sn[id]->start? (sn[id]->cur_pos - sn[id]->start)
		:(sn[id]->cur_pos + pool_size - sn[id]->start);
*/	
	if(snull_removed)
	{
//		spin_unlock(&priv->lock);
		return;
	}
redo:	
	while(sn[id]->cur_pos != sn[id]->start)
	{
		snull_rx(dev, &tx_snull_pkt[id][sn[id]->start]);
	}

	sn[id]->flag = 0;
	if(sn[id]->cur_pos != sn[id]->start)
		goto redo;

/*	if(!num)
	{
		spin_unlock(&priv->lock);
		tasklet_schedule(&starg->tlet);
	}
	else
	{
		index = 0;
		while(index < num)
		{
			snull_rx(dev, &tx_snull_pkt[id][sn[id]->start]);
			index ++;
		}
		spin_unlock(&priv->lock);
		tasklet_schedule(&starg->tlet);
	}
*/
}

int tx_size = 32;
int curr_tx = 0;
int tx_free = 32;

static inline struct snull_packet *snull_get_rx_pk(struct packet_control *ctl, int idx)
{
	struct snull_packet *pkt;

	if(((ctl->cur_pos + 1) % pool_size) == ctl->start)
		return 0;

	pkt = &tx_snull_pkt[idx][ctl->cur_pos];
	return pkt;
}

static int snull_real_tx_opt(char *src, int idx, int len)
{
	struct snull_packet *des;
	struct packet_control *ctl;

	ctl = sn[idx];
	//__raw_spin_lock((raw_spinlock_t *) &ctl->lock);
	des = snull_get_rx_pk(ctl, idx);
	if(!des)
	{
		//__raw_spin_unlock((raw_spinlock_t *) &ctl->lock);
		return -1;
	}
	memcpy(des->data, src, len);
	des->datalen = len;
	if(ctl->cur_pos == pool_size - 1)
		ctl->cur_pos = 0;
	else
		ctl->cur_pos ++;
	//__raw_spin_unlock((raw_spinlock_t *)&ctl->lock);
	return 0;
}

/*
 * Transmit a packet (low level interface)
 */
static inline void snull_hw_tx(char *buf, int len, struct net_device *dev)
{
	/*
	 * This function deals with hw details. This interface loops
	 * back the packet to the other snull interface (if any).
	 * In other words, this function implements the snull behaviour,
	 * while all other procedures are rather device-independent
	 */
	struct iphdr *ih;
	struct snull_priv *priv;
	struct ethhdr *eth;
	struct pcpu_lstats *pcpu_lstats, *lb_stats;
	unsigned char des;
	unsigned char *saddr, *daddr, *mac;
	unsigned char multi;
	int i;

	/* I am paranoid. Ain't I? */
	if (unlikely(len < sizeof(struct ethhdr) + sizeof(struct iphdr))) {
		printk("snull: Hmm... packet too short (%i octets)\n",
				len);
		return;
	}
	//printk(KERN_INFO "Dom 0: has reached snull_hw_tx\n");
	/* enable this conditional to look at the data */
	
	if (0) { 		int i;
		PDEBUG("len is %i\n" KERN_DEBUG "data:",len);
		for (i=14 ; i<len; i++)
			printk(" %02x",buf[i]&0xff);
		printk("\n");
	}

	/*
	 * Ethhdr is 14 bytes, but the kernel arranges for iphdr
	 * to be aligned (i.e., ethhdr is unaligned)
	 */
	eth = (struct ethhdr *) buf;
	ih = (struct iphdr *)(buf+sizeof(struct ethhdr));
		
	ih->check = 0;         /* and rebuild the checksum (ip needs it) */
	ih->check = ip_fast_csum((unsigned char *)ih,ih->ihl);
	priv = netdev_priv(dev);

	saddr = (unsigned char *)&ih->saddr;
	daddr = (unsigned char *)&ih->daddr;
	mac = (unsigned char *)&eth->h_dest;
	//printk(KERN_INFO "Dom0 send: %d.%d.%d.%d\n", daddr[0], daddr[1], daddr[2], daddr[3]);

	des = daddr[3] - 1;
	multi = daddr[0];
	if(daddr[3] == 254 || (multi >= 224 && multi <= 238))
	{
		for(i = 0; i < MAX_DOMAINS; i ++)
		{
			{
				des = i;
				if(likely(sn[id]->ready && sn[des]->ready))
				{
					__raw_spin_lock((raw_spinlock_t *) &sn[des]->lock);
					snull_real_tx_opt(buf, des, len);			
					__raw_spin_unlock((raw_spinlock_t *) &sn[des]->lock);
					if(sn[des]->flag == 0 && sn[des]->ready == 1){
						sn[des]->flag = 1;
						//send_dataready_ipi(des*4 + 4);
						//R715 has no offset of apicid.
						send_dataready_ipi(des);
					}
				}
				if(sn[des]->ready == 0){// reset all variables.
					sn[des]->flag = 0;
					sn[des]->lock = 0;
					sn[des]->cur_pos = sn[des]->start = 0;
				}
			}
			
		}
	
	}
	else
	{
		if(des < MAX_DOMAINS)
		{
			if(likely(sn[id]->ready && sn[des]->ready))
			{
				__raw_spin_lock((raw_spinlock_t *) &sn[des]->lock);
				snull_real_tx_opt(buf, des, len);			
				__raw_spin_unlock((raw_spinlock_t *) &sn[des]->lock);
				if(sn[des]->flag == 0 && sn[des]->ready == 1){
					sn[des]->flag = 1;
					//send_dataready_ipi(des*4 + 4);
					//R715
					send_dataready_ipi(des);
				}
			}
			if(sn[des]->ready == 0){// reset all variables.
				sn[des]->flag = 0;
				sn[des]->lock = 0;
				sn[des]->cur_pos = sn[des]->start = 0;
			}
		}
	}

	priv->tx_packetlen = len;
	priv->tx_packetdata = buf;

	dev_kfree_skb(priv->skb);

	pcpu_lstats = priv->lstats;
	lb_stats = per_cpu_ptr(pcpu_lstats, smp_processor_id());
	lb_stats->tbytes += len;
	lb_stats->tpackets ++;
}

/*
 * Transmit a packet (called by the kernel)
 */
int snull_tx(struct sk_buff *skb, struct net_device *dev)
{
	int len;
	char *data;
	struct snull_priv *priv = netdev_priv(dev);
	
	//printk(KERN_INFO "Send a packet\n");
	//printk(KERN_INFO "Dom0: has reached snull_tx\n");
	data = skb->data;
	len = skb->len;
	/*
	if (len < ETH_ZLEN) {
		memset(shortpkt, 0, ETH_ZLEN);
		memcpy(shortpkt, skb->data, skb->len);
		len = ETH_ZLEN;
		data = shortpkt;
	}
	*/
	dev->trans_start = jiffies; /* save the timestamp */

	/* Remember the skb, so we can free it at interrupt time */
	priv->skb = skb;

	/* actual deliver of data is device-specific, and not shown here */
	snull_hw_tx(data, len, dev);

	return 0; /* Our simple device can not fail */
}

/*
 * Deal with a transmit timeout.
 */
void snull_tx_timeout (struct net_device *dev)
{
	struct snull_priv *priv = netdev_priv(dev);
	struct pcpu_lstats *pcpu_lstats, *lb_stats;

	PDEBUG("Transmit timeout at %ld, latency %ld\n", jiffies,
			jiffies - dev->trans_start);
        /* Simulate a transmission interrupt to get things moving */
	priv->status = SNULL_TX_INTR;
	
	pcpu_lstats = priv->lstats;
	lb_stats = per_cpu_ptr(pcpu_lstats, smp_processor_id());
	lb_stats->tbytes += priv->tx_packetlen;
	lb_stats->tpackets ++;

	dev_kfree_skb(priv->skb);
	netif_wake_queue(dev);
	return;
}

/*
 * Ioctl commands 
 */
int snull_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
	PDEBUG("ioctl\n");
	return 0;
}

/*
 * Return statistics to the caller
 */
struct net_device_stats *snull_stats(struct net_device *dev)
{
	struct snull_priv *priv = netdev_priv(dev);
	const struct pcpu_lstats *pcpu_lstats;
	struct net_device_stats *stats = &priv->stats;
	int i;

	stats->rx_packets = 0;
	stats->tx_packets = 0;
	stats->rx_bytes = 0;
	stats->tx_bytes = 0;


	pcpu_lstats = priv->lstats;
	for_each_possible_cpu(i)
	{
		const struct pcpu_lstats *lb_stats;
		lb_stats = per_cpu_ptr(pcpu_lstats, i);
		stats->rx_packets += lb_stats->rpackets;
		stats->tx_packets += lb_stats->tpackets;
		stats->rx_bytes += lb_stats->rbytes;
		stats->tx_bytes += lb_stats->tbytes;
	}

	return stats;
}

/*
 * This function is called to fill up an eth header, since arp is not
 * available on the interface
 */
int snull_rebuild_header(struct sk_buff *skb)
{
	struct ethhdr *eth = (struct ethhdr *) skb->data;
	struct net_device *dev = skb->dev;
    
	memcpy(eth->h_source, dev->dev_addr, dev->addr_len);
	memcpy(eth->h_dest, dev->dev_addr, dev->addr_len);
	eth->h_dest[ETH_ALEN-1]   += id;   /* dest is us add domain id */
	return 0;
}

int snull_header(struct sk_buff *skb, struct net_device *dev,
		unsigned short type, const void *daddr, const void *saddr,
		unsigned int len)
{
	struct ethhdr *eth = (struct ethhdr *)skb_push(skb,ETH_HLEN);
	struct iphdr *ih;
	unsigned char des;
	unsigned char *dadd;
	

	ih = (struct iphdr*)(skb->data + sizeof(struct ethhdr));
	des = ((unsigned char *)&ih->daddr)[3] - 1; /* MAC addr has the id added */
	dadd = (unsigned char *)&ih->daddr;
	
	if(des >= MAX_DOMAINS)
		des = 0;
	
	eth->h_proto = htons(type);
	memcpy(eth->h_source, saddr ? saddr : dev->dev_addr, dev->addr_len);
	memcpy(eth->h_dest,   daddr ? daddr : dev->dev_addr, dev->addr_len);

	{
		eth->h_dest[ETH_ALEN - 1] = 0x0;
		eth->h_dest[ETH_ALEN - 1] = 0x30 + des;   /* dest is us add domain distance */
	}

	if(dadd[3] == 255)
	{
		int i;
		for(i =0; i < ETH_HLEN; i ++)
			eth->h_dest[i] = 0xFF;
	}
	if((dadd[0] & 0xe0) == 0xe0)
	{
		eth->h_dest[0] = 0x01;
		eth->h_dest[1] = 0x0;
		eth->h_dest[2] = 0x5e;
		eth->h_dest[3] = 0x7f & dadd[1];
		eth->h_dest[4] = dadd[2];
		eth->h_dest[5] = dadd[3];
	}
	
	return (dev->hard_header_len);
}

/*
 * The "change_mtu" method is usually not needed.
 * If you need it, it must be like this.
 */
int snull_change_mtu(struct net_device *dev, int new_mtu)
{
	unsigned long flags;
	struct snull_priv *priv = netdev_priv(dev);
	spinlock_t *lock = &priv->lock;
    
	/* check ranges */
	if ((new_mtu < 68) || (new_mtu > 1500))
		return -EINVAL;
	/*
	 * Do anything you need, and the accept the value
	 */
	spin_lock_irqsave(lock, flags);
	dev->mtu = new_mtu;
	spin_unlock_irqrestore(lock, flags);
	return 0; /* success */
}

/*
 * The init function (sometimes called probe).
 * It is invoked by register_netdev()
 */
static const struct header_ops snull_header_ops = 
{
	.create = snull_header,
	.parse  = NULL,
	//.rebuild = snull_rebuild_header,
	.cache = NULL,
	.cache_update = NULL,
};

static const struct net_device_ops snull_ops = 
{
	.ndo_open = snull_open,
	.ndo_stop = snull_release,
	.ndo_set_config = snull_config,
	.ndo_start_xmit = snull_tx,
	.ndo_do_ioctl = snull_ioctl,
	.ndo_get_stats = snull_stats,
	.ndo_change_mtu = snull_change_mtu,
	.ndo_tx_timeout = snull_tx_timeout,
};

void snull_init(struct net_device *dev)
{
	struct snull_priv *priv;
	struct pcpu_lstats *lstats;
#if 0
	/*
	 * Make the usual checks: check_region(), probe irq, ...  -ENODEV
	 * should be returned if no device found.  No resource should be
	 * grabbed: this is done on open(). 
	 */
#endif

	/* 
	 * Then, assign other fields in dev, using ether_setup() and some
	 * hand assignments
	 */
	ether_setup(dev); /* assign some of the fields */
	dev->netdev_ops = &snull_ops;
	dev->header_ops = &snull_header_ops;
	dev->destructor = snull_dev_free;
	
	/* keep the default flags, just add NOARP */
	dev->flags           |= IFF_NOARP;
//	dev->features        |= NETIF_F_NO_CSUM;

	/*
	 * Then, initialize the priv field. This encloses the statistics
	 * and a few private fields.
	 */
	priv = netdev_priv(dev);
	memset(priv, 0, sizeof(struct snull_priv));
	lstats = alloc_percpu(struct pcpu_lstats);
	priv->lstats = lstats;
	spin_lock_init(&priv->lock);
	//printk(KERN_INFO "snull_init()\n");
}

/*
 * The devices
 */

struct net_device *snull_dev;

/*
 * Finally, the module stuff
 */
void snull_cleanup(void)
{
	int i;
	if (snull_dev) {
		free_irq(IRQ_NUM, NULL);
		unregister_netdev(snull_dev);
		free_netdev(snull_dev);
		kfree(sn[0]);
		for(i = 0; i < MAX_DOMAINS; i ++)
			kfree(tx_snull_pkt[i]);
		printk(KERN_INFO "Has unregisterd snull\n");
	}
	kfree(stl);
	snull_removed = 1;
	printk(KERN_INFO "Has removed snull\n");
	return;
}

static int __init snull_init_module(void)
{
	int result, ret = -ENOMEM;
	int pages;
	int j;
	int irqr;
	struct packet_control *ctmp;
	uint64_t shared_addr, buf_addr;
	
	id = vmm_get_did();
	id = 0;

	shared_addr = vmm_snull_init(0, 0);

	pages = (sizeof(struct snull_packet) * pool_size)%4096 ? sizeof(struct snull_packet) * pool_size/4096 : (sizeof(struct snull_packet) * pool_size/4096 + 1);

	ctmp = (struct packet_control *)ioremap(shared_addr, 100*1024*1024);
	if(!ctmp){
		printk(KERN_INFO "ioremap failed:%lx\n", shared_addr);
		return -1;
	}
	buf_addr = ((uint64_t)ctmp) + 4096;


	for(j = 0; j < 2; j ++)
		sn[j] = &ctmp[j];
	
	for(j = 0; j < 2; j ++)
	{
		sn[j]->pkt = (struct snull_packet*)(buf_addr + pages*4096*j);
		sn[j]->p_pkt = (struct snull_packet*)(shared_addr+4096+pages*4096*j);
		sn[j]->ready = 0;
		sn[j]->lock = 0;
		sn[j]->cur_pos = sn[j]->start = 0;
		sn[j]->flag = 0;
	}

	for(j = 0; j < MAX_DOMAINS; j ++)
		tx_snull_pkt[j] = sn[j]->pkt;

	sn[id]->cur_pos = sn[id]->start = 0;
	sn[id]->lock = 0;
	sn[id]->ready = 1;
	
	/*
	sn->pkt = (struct snull_packet*)kmalloc(pages * 4096, GFP_KERNEL);
	sn->p_pkt = (struct snull_packet*)virt_to_phys((void *)sn->pkt);
	sn->ready = 1;
	
	sn1 = &sn[1];
	
	sn1->tpkt = (struct snull_packet*)kmalloc(pages * 4096, GFP_KERNEL);
	if(sn1->tpkt)
		printk("sn1 packet  addr is 0x%llx\n", (u64)sn1->tpkt);
	sn1->p_pkt = (struct snull_packet*)virt_to_phys((void *)sn1->tpkt);
	printk("sn1 packet phy addr is 0x%llx\n", (u64)sn1->p_pkt);
	*/
		
	snull_dev = alloc_netdev(sizeof(struct snull_priv), "sn0",
									 snull_init);
	if (snull_dev == NULL)
		goto out;

	ret = -ENODEV;
	if ((result = register_netdev(snull_dev)))
		printk("snull: error %i registering device \"%s\"\n",
				 result, snull_dev->name);
	else
		ret = 0;
	irqr = request_irq(IRQ_NUM, ipi_handler, IRQF_DISABLED, "snull_irq", NULL);
	if(irqr)
		printk(KERN_INFO "request_irq fialed:%d\n", irqr);
	//printk(KERN_INFO "irqr: %d\n", irqr);

//	printk(KERN_INFO "Pages :%d, addr:%lx\n", pages, virt_to_phys(sn[0]));


	stl = (struct snull_tlet_arg *)kmalloc(sizeof(struct snull_tlet_arg), GFP_KERNEL);

	stl->dev = snull_dev;
	tasklet_init(&stl->tlet, snull_tasklet_rx, (unsigned long)stl);
//	tasklet_schedule(&stl->tlet);
   out:
	if (ret) 
		snull_cleanup();
	return ret;
}

module_init(snull_init_module);
module_exit(snull_cleanup);
