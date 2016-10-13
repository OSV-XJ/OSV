/*
 * INET		An implementation of the TCP/IP protocol suite for the LINUX
 *		operating system.  INET is implemented using the  BSD Socket
 *		interface as the means of communication with the user level.
 *
 *		Pseudo-driver for the loopback interface.
 *
 * Version:	@(#)loopback.c	1.0.4b	08/16/93
 *
 * Authors:	Ross Biro
 *		Fred N. van Kempen, <waltje@uWalt.NL.Mugnet.ORG>
 *		Donald Becker, <becker@scyld.com>
 *
 *		Alan Cox	:	Fixed oddments for NET3.014
 *		Alan Cox	:	Rejig for NET3.029 snap #3
 *		Alan Cox	: 	Fixed NET3.029 bugs and sped up
 *		Larry McVoy	:	Tiny tweak to double performance
 *		Alan Cox	:	Backed out LMV's tweak - the linux mm
 *					can't take it...
 *              Michael Griffith:       Don't bother computing the checksums
 *                                      on packets received on the loopback
 *                                      interface.
 *		Alexey Kuznetsov:	Potential hang under some extreme
 *					cases removed.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 */
#include <linux/kernel.h>
#include <linux/jiffies.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/socket.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/in.h>
#include <linux/init.h>

#include <asm/system.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#include <linux/inet.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/ethtool.h>
#include <net/sock.h>
#include <net/checksum.h>
#include <linux/if_ether.h>	/* For the statistics structure. */
#include <linux/if_arp.h>	/* For ARPHRD_ETHER */
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/percpu.h>
#include <net/net_namespace.h>

struct snull_priv {
	struct net_device_stats stats;
	int status;
	int rx_int_enabled;
	int tx_packetlen;
	u8 *tx_packetdata;
	struct sk_buff *skb;
	spinlock_t lock;
};

struct snull_packet {
	struct snull_packet *next;
	struct net_device *dev;
	int	datalen;
	u8 data[ETH_DATA_LEN];
	int free;
};

struct snull_packet *gtx_buff;
int tx_size = 32;
int curr_tx = 0;
int tx_free = 32;

static struct snull_packet *loopback_get_txbuff(void)
{
	struct snull_packet *pkt = NULL;
	if(tx_free)
	{
		do
		{
			pkt = &gtx_buff[curr_tx];
			curr_tx %= tx_size;
		}while(!pkt->free);
		tx_free --;
		pkt->free = 0;
	}
	return pkt;
}

static int loopback_release_tkbuff(struct snull_packet *pkt) 
{
	tx_free ++;
	pkt->free = 1;
	return tx_free;
}

/*
 * The higher levels take care of making this non-reentrant (it's
 * called with bh's disabled).
 */
static void snull_hw_tx(char *buf, int len, struct net_device *dev)
{
	/*
	 * This function deals with hw details. This interface loops
	 * back the packet to the other snull interface (if any).
	 * In other words, this function implements the snull behaviour,
	 * while all other procedures are rather device-independent
	 */
	struct iphdr *ih;
	struct snull_priv *priv;
	u32 *saddr, *daddr;
	struct snull_packet *tx_buffer;
    
	/* I am paranoid. Ain't I? */
	if (len < sizeof(struct ethhdr) + sizeof(struct iphdr)) {
		printk("snull: Hmm... packet too short (%i octets)\n",
				len);
		return;
	}

	/*
	 * Ethhdr is 14 bytes, but the kernel arranges for iphdr
	 * to be aligned (i.e., ethhdr is unaligned)
	 */
	ih = (struct iphdr *)(buf+sizeof(struct ethhdr));
	saddr = &ih->saddr;
	daddr = &ih->daddr;

	ih->check = 0;         /* and rebuild the checksum (ip needs it) */
	ih->check = ip_fast_csum((unsigned char *)ih,ih->ihl);
	priv = netdev_priv(dev);

	spin_lock_irq(&priv->lock);

	tx_buffer = loopback_get_txbuff();
	
	tx_buffer->datalen = len;
	memcpy(tx_buffer->data, buf, len);

	//vmm_snull_tx((u64)virt_to_phys((void *)tx_buffer), id);

	priv->tx_packetlen = len;
	priv->tx_packetdata = buf;
	priv->stats.tx_packets++;
	priv->stats.tx_bytes += priv->tx_packetlen;
	dev_kfree_skb(priv->skb);
	loopback_release_tkbuff(tx_buffer);
	spin_unlock_irq(&priv->lock);
}


static int loopback_xmit(struct sk_buff *skb, struct net_device *dev)
{
	int len;
	char *data, shortpkt[ETH_ZLEN];
	struct snull_priv *lb_stats = netdev_priv(dev);
	
	data = skb->data;
	len = skb->len;
	if (len < ETH_ZLEN) {
		memset(shortpkt, 0, ETH_ZLEN);
		memcpy(shortpkt, skb->data, skb->len);
		len = ETH_ZLEN;
		data = shortpkt;
	}
	dev->trans_start = jiffies; /* save the timestamp */

	/* Remember the skb, so we can free it at interrupt time */
	lb_stats->skb = skb;

	/* actual deliver of data is device-specific, and not shown here */
	snull_hw_tx(data, len, dev);

	return 0; /* Our simple device can not fail */
}

static struct net_device_stats *loopback_get_stats(struct net_device *dev)
{
	struct snull_priv *lb_stats = netdev_priv(dev);
	return &lb_stats->stats;
}

static u32 always_on(struct net_device *dev)
{
	return 1;
}

static const struct ethtool_ops loopback_ethtool_ops = {
	.get_link		= always_on,
	.set_tso		= ethtool_op_set_tso,
	.get_tx_csum		= always_on,
	.get_sg			= always_on,
	.get_rx_csum		= always_on,
};

static int loopback_dev_init(struct net_device *dev)
{
	struct snull_priv *lstats;

	lstats = netdev_priv(dev);
	if (!lstats)
		return -ENOMEM;

	spin_lock_init(&lstats->lock);
	return 0;
}

static void loopback_dev_free(struct net_device *dev)
{
	free_netdev(dev);
}

/*
 * Open and close
 */

static int loopback_open(struct net_device *dev)
{
	/* request_region(), request_irq(), ....  (like fops->open) */

	/* 
	 * Assign the hardware address of the board: use "\0SNULx", where
	 * x is 0 or 1. The first byte is '\0' to avoid being a multicast
	 * address (the first byte of multicast addrs is odd).
	 */
	memcpy(dev->dev_addr, "\0SNUL0", ETH_ALEN);
	netif_start_queue(dev);
	printk(KERN_INFO"here in snull open \n");
	return 0;
}

static const struct net_device_ops loopback_ops = {
	.ndo_open 		= loopback_open,
	.ndo_init      = loopback_dev_init,
	.ndo_start_xmit= loopback_xmit,
	.ndo_get_stats = loopback_get_stats,
};

/*
 * The loopback device is special. There is only one instance
 * per network namespace.
 */
static void loopback_setup(struct net_device *dev)
{
	ether_setup(dev);
	dev->mtu		= (16 * 1024) + 20 + 20 + 12;
	dev->hard_header_len	= ETH_HLEN;	/* 14	*/
	dev->addr_len		= ETH_ALEN;	/* 6	*/
	dev->flags		= IFF_NOARP;
	dev->features 		= NETIF_F_NO_CSUM;
	dev->ethtool_ops	= &loopback_ethtool_ops;
	dev->netdev_ops		= &loopback_ops;
	dev->destructor		= loopback_dev_free;
}

static struct net_device *sn;
/* Setup and register the loopback device. */
static int __init loopback_net_init(void)
{
	int i, err;

	err = -ENOMEM;
	sn = alloc_netdev(sizeof(struct snull_priv), "sn0", loopback_setup);
	if (!sn)
		goto out;

	gtx_buff = (struct snull_packet *)kmalloc(sizeof(struct snull_packet)*tx_size, GFP_KERNEL);
	for(i = 0; i < tx_size; i ++)
		gtx_buff[i].free = 1;
	err = register_netdev(sn);
	if (err)
		goto out_free_netdev;

	return 0;


out_free_netdev:
	free_netdev(sn);
out:
	return err;
}

static void __exit loopback_net_exit(void)
{
	kfree(gtx_buff);
	unregister_netdev(sn);
}

module_init(loopback_net_init);
module_exit(loopback_net_exit)
