//#include <linux/socket.h>
#include "osv_vmm.h"


struct osv_skb{
	struct osv_skb * next;
	struct osv_addr sour;
	struct osv_buffer *msg;
	int port, offset, len;
	char type;
};

struct osv_skb_head{
	struct osv_skb * next;
	struct osv_skb * tail;
//	atomic_t len;
//	spinlock_t skb_lock;
	struct sem len;
	struct sem skb_lock;
};

/*struct osv_sock{
	struct sock	sk;
	struct osv_addr dect;
	struct osv_addr	sour;
//	char	*recv_buf;
	struct osv_skb_head skb_head;
	int (*data_ready) (struct osv_sock* osk, struct osv_buffer * msg);
	spinlock_t 	lock;
};*/

/*struct domain_recv_buff{
	char *	head;	// This is the domain receive buffer head point
	int 	avail_space; 	// The availiable space in the 
				//domain receive buffer for writting.
	int 	write_pos;	// The position for writting.
	int 	read_pos;	// The position for reading.
	int 	msg_num;
	int 	write_pos_back;
	spinlock_t	write_lock;	
	spinlock_t	num_lock;	
}__attribute__((packed));
*/
struct osv_recv_msg{
	int 	msg_len;
	struct osv_addr	sour;	
	int 	dest_port;
	int 	flags;
	char	type;
	struct osv_buffer *msg_data;
};

struct domain_buf_control{
	int start;
	int space_index[TAB_ENTRYS];
//	spinlock_t buf_lock;
	struct sem buf_lock;
	int free_index;
	long msg_order[TAB_ENTRYS];
	int msg_index;
	struct sem recv_lock;
//	spinlock_t recv_lock;
	int end;
	struct osv_recv_msg msg[RECV_BUFF_LEN/BLOCK_SIZE];
	int flag;
};
