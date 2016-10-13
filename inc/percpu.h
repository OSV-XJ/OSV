#ifndef PERCPU_HEADER
#define PERCPU_HEADER
#include <inc/types.h>

/*
 * Linux like per cpu data definitions.
 * Codes are borrowed from Linux.
 */

#define DEFINE_PER_CPU(type, name)					\
	typeof(type) per_cpu_##name __attribute__((__section__(".data.percpu")))

struct osv_pda
{
	uint64_t offset;
	uint64_t cpulid;
	uint64_t cpupid;
	uint64_t cpudid;
	uint64_t vmcb_addr;
} __attribute__((aligned(64)));

extern struct osv_pda pda[32];
static struct osv_pda __proxy_pda;

#define pda_offset(field) offsetof(struct osv_pda, field)

#define pda_to_op(op, field, val)					\
	do {														\
		typedef typeof(__proxy_pda.field) __T;		\
		switch(sizeof(__proxy_pda.field))			\
		{														\
		case 2:												\
			__asm__(op"w %1, %%gs:%c2":				\
					  "+m"(__proxy_pda.field):			\
					  "ri"((__T)val),						\
					  "i"(pda_offset(field)));			\
			break;											\
		case 4:												\
			__asm__(op"l %1, %%gs:%c2":				\
					  "+m"(__proxy_pda.field):			\
					  "ri"((__T)val),						\
					  "i"(pda_offset(field)));			\
			break;											\
		case 8:												\
			__asm__(op"q %1, %%gs:%c2":				\
					  "+m"(__proxy_pda.field):			\
					  "ri"((__T)val),						\
					  "i"(pda_offset(field)));			\
			break;											\
		}														\
	} while(0)



#define pda_from_op(op, field)						\
	({															\
		typeof(__proxy_pda.field) ret__;				\
		ret__ = 0;											\
		switch(sizeof(__proxy_pda.field))			\
		{														\
		case 2:												\
			__asm__(op"w %%gs:%c1, %0":				\
					  "=r"(ret__):							\
					  "i"(pda_offset(field)),			\
					  "m"(__proxy_pda.field));			\
			break;											\
		case 4:												\
			__asm__(op"l %%gs:%c1, %0":				\
					  "=r"(ret__):							\
					  "i"(pda_offset(field)),			\
					  "m"(__proxy_pda.field));			\
			break;											\
		case 8:												\
			__asm__(op"q %%gs:%c1, %0":				\
					  "=r"(ret__):							\
					  "i"(pda_offset(field)),			\
					  "m"(__proxy_pda.field));			\
			break;											\
		}														\
		ret__;												\
	})

#define read_pda(field)  pda_from_op("mov", field)
#define write_pda(field, val)  pda_to_op("mov", field, val)

int percpu_init(void);
int cpu_init(void);
#endif
