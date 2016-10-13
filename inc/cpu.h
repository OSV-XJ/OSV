#include <inc/types.h>

#define MAX_CPU 32

struct cpu
{
	unsigned int lapicid;
	unsigned int cpuid;
	unsigned int nodeid;
	unsigned int bootp;
	unsigned int logicalid;
	volatile int dsvm;
	volatile int svm;
	volatile int booted;
}__attribute__((__packed__));

volatile extern struct cpu cpus[MAX_CPU];
uint8_t lapicid_to_index[256];
extern uint64_t cpuid_offset;
extern uint64_t cpunum;





