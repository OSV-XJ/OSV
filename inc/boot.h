#define	DESCRIPTOR(base,limit,attr)	\
	.word	limit		&	0x0000FFFF;	\
	.word	base		&	0x000000000000FFFF;	\
	.byte	base>>16		&	0x00000000000000FF;	\
	.byte	attr		&	0x00000000000000FF;	\
	.byte	((attr>>4)&0x00000000000000F0)|((limit>>16)&0x000000000000000F);	\
	.byte	base>>24		&	0x00000000000000FF

#define	DESCRIPTOR64(attr) DESCRIPTO(0,0,attr)
#define	ATTR_G		0x800
#define	ATTR_DB	0x400

#define	ATTR_L		0x200

#define	ATTR_AVL	0x100
#define	ATTR_P		0x080

#define	ATTR_DPL(level)	((level & 0x03)<<5)
#define	ATTR_S		0x010


#define	ATTR_TYPE(type)	(type & 0x00f)


#define	ATTR_TYPE_READ_UPWARD_DATA		0x000
#define	ATTR_TYPE_READ_DOWNWARD_DATA	0x004
#define	ATTR_TYPE_RW_UPWARD_DATA		0x002
#define	ATTR_TYPE_RW_DOWNWARD_DATA	0x006
#define	ATTR_TYPE_EXE_CONFIRM_CODE		0x00c		


#define	ATTR_TYPE_EXE_UNCONFIRM_CODE	0x008
#define	ATTR_TYPE_READ_CONFIRM_CODE	0x00e
#define	ATTR_TYPE_READ_UNCONFIRM_CODE	0x00a
#define	ATTR_TYPE_ACCESSED			0x001


#define	ATTR_TYPE_286		0x008
#define	ATTR_TYPE_TSS		0x001
#define	ATTR_TYPE_LDT		0x002		

#define	ATTR_TYPE_BUSY_TSS	0x003
#define	ATTR_TYPE_CALL_GATE	0x004
#define	ATTR_TYPE_INT_GATE	0x006
#define	ATTR_TYPE_TRAP_GATE	0x007

/*********** GDTR ***************/

#define	GDTR(addr, limit)		\
	.word	limit;	\
	.long	addr

/*********** SELECTOR ************/

#define	SELECTOR(index, ti, rpl) (((index<<3) & 0xfff8) | ((ti<<2) & 0x4) | (rpl & 0x3))

#define toCR3(base_addr,PCD,PWT) (\
	(base_addr && 0x000000fffffff000) |\
	((PCD<<4) & 0x10)	|\
	((PWT<<3) & 0x8))
/********** CR3 ******************/
#define toCR3_normal(base_addr) (base_addr & 0x000000fffffff000)

/*********** paging **************/
#define PAGE_P	0x01	/*present*/
#define PAGE_RW	0x02	/*read/write*/
#define PAGE_US	0x04	/*user/superisor*/
#define PAGE_PWT	0x08
#define PAGE_PCD	0x10
#define PAGE_A	0x20	/*available*/
#define	PAGE_D	0x40
#define PAGE_PAT	0x80
#define	PAGE_G	0x100

#define PML4_ENTRY(base_addr,attr)	((base_addr & 0x000000FFFFFFF000)|(attr & 0x03f))
#define PDPT_ENTRY(base_addr,attr)	PML4_ENTRY(base_addr,attr)
#define PDT_ENTRY(base_addr,attr)	PML4_ENTRY(base_addr,attr)
#define	PT_ENTRY(base_addr,attr)	((base_addr & 0x000000FFFFFFF000)|(attr & 0x1ff))

