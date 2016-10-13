#ifndef __OSV_VMM_H__
#define __OSV_VMM_H__

#define RECV_BUFF_BASE	0x
#define RECV_BUFF_LEN	(1024*1024*8)
#define RECV_TABLE_BASE	0xXXXX
#define RECV_TABLE_N		0xXXXX
#define SCHEDULE_TIMEOUT	5
#define MAX_DOMAIN_NUM	2
#define BLOCK_SIZE (1024*8)
#define TAB_ENTRYS	(RECV_BUFF_LEN/BLOCK_SIZE)


struct osv_buffer {
	char * addr;
//	int    offset;
	int    len;
};

struct osv_addr {
	int 	ip;
	int 	port;
};

struct osv_table_entry {
	int 	port;	// referrence to a process.
	int 	status;
#define OSV_STATUS_FREE     0
#define OSV_STATUS_INUSE	1
	int 	timestamp;
	struct osv_addr		srcaddr;
	struct osv_buffer	buff;
	char 	type;
};

struct osv_table {
    struct osv_table_entry * ot_table;
    int ot_len;
};

/**
* transfer. send data to the destination domain.
* design:
*	every domain has only one recv buff and one recv table but several send tables.
*	when a domain send something to another domain, it will call the function 
*	'transfer' to tell the vmm where the data located with parameter 'buff' and which
*	domain will recv the data. and then vmm copy the data to the recv buff of the dest
*	domain. at the same time, vmm has to add an entry to the recv table.
*
*	the dest domain will check the recv table to find whether a request is comming.
*
* @buff	data to be transmitted.(physical addr and data length)
* @destaddr	destination address.(including ip and port)
* @srcaddr	source address(including ip and receive buffer port)
* 
* entry values:
* 	@port: destaddr.port
*	@status: OSV_STATUS_UNPROCESSED
*	@timestamp:	current tickcount
*	@buff.addr:	where the recved data located.
*	@buff.len:	data length.
*/
static int transfer(void * buff, struct osv_addr *destaddr, struct osv_addr *srcaddr, char type);

/**
* listen. tell the monitor that a process is listening with port destaddr->port.
*/
//int (struct osv_addr)

/**
* vmm_init_osv. tell the vmm where the recv-buff and recv-table are located.
* @recv_buff	recv_buff information
* @head	recv_table base addr.
* @osv_table_len	number of entries in recv_table.
*/
int vmm_init_osv(struct osv_buffer recv_buff, struct osv_table table);
int vmm_exit_osv(void);

#endif
