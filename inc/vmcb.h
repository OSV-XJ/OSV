#ifndef VMCB_HEADER
#define VMCB_HEADER

#include <inc/types.h>


/*general intercepts*/

enum Interceptbits1
{
	INTERCEPT_INTR = 1 << 0,
	INTERCEPT_NMI = 1 << 1,
	INTERCEPT_SMI = 1 << 2,
	INTERCEPT_INIT = 1 << 3,
	INTERCEPT_VINTR = 1 << 4,
	INTERCEPT_CR0_SEL_WRITE = 1 << 5,
	INTERCEPT_IDTR_READ = 1 << 6,
	INTERCEPT_GDTR_READ = 1 << 7,
	INTERCEPT_LDTR_READ = 1 << 8,
	INTERCEPT_TR_READ = 1 << 9,
	INTERCEPT_IDTR_WRITE = 1 << 10,
	INTERCEPT_GDTR_WRITE = 1 << 11,
	INTERCEPT_LDTR_WRITE = 1 << 12,
	INTERCEPT_TR_WRITE = 1 << 13,
	INTERCEPT_RDTSC = 1 << 14,
	INTERCEPT_RDPMC = 1 << 15,
	INTERCEPT_PUSHF = 1 << 16,
	INTERCEPT_POPF = 1 << 17,
	INTERCEPT_CPUID = 1 << 18,
	INTERCEPT_RSM = 1 << 19,
	INTERCEPT_IRET = 1 << 20,
	INTERCEPT_SWINT = 1 << 21,
	INTERCEPT_INVD = 1 << 22,
	INTERCEPT_PAUSE = 1 << 23,
	INTERCEPT_HLT = 1 << 24,
	INTERCEPT_INVLPG = 1 << 25,
	INTERCEPT_INVLPGA = 1 << 26,
	INTERCEPT_IOIO_PROT = 1 << 27,
	INTERCEPT_MSR_PROT = 1 << 28,
	INTERCEPT_TASK_SWITCH = 1 << 29,
	INTERCEPT_FERR_FREEZE = 1 << 30,
	INTERCEPT_SHUTDOWN_EVT = 1 << 31
};

enum Interceptbits2
{
	INTERCEPT_VMRUN = 1 << 0,
	INTERCEPT_VMCALL = 1 << 1,
	INTERCEPT_VMLOAD = 1 << 2,
	INTERCEPT_VMSAVE = 1 << 3,
	INTERCEPT_STGI = 1 << 4,
	INTERCEPT_CLGI = 1 << 5,
	INTERCEPT_SKINIT = 1 << 6,
	INTERCEPT_RDTSCP = 1 << 7,
	INTERCEPT_ICEBP = 1 << 8,
	INTERCEPT_WBINVD = 1 << 9,
	INTERCEPT_MONITOR = 1 << 10,
	INTERCEPT_MWAIT = 1 << 11,
	INTERCEPT_MWAIT_CONDITIOANL = 1 << 12
};

enum CRInterceptBits
{
	CR_INTERCEPT_CR0_READ = 1 << 0,
	CR_INTERCEPT_CR1_READ = 1 << 1,
	CR_INTERCEPT_CR2_READ = 1 << 2,
	CR_INTERCEPT_CR3_READ = 1 << 3,
	CR_INTERCEPT_CR4_READ = 1 << 4,
	CR_INTERCEPT_CR5_READ = 1 << 5,
	CR_INTERCEPT_CR6_READ = 1 << 6,
	CR_INTERCEPT_CR7_READ = 1 << 7,
	CR_INTERCEPT_CR8_READ = 1 << 8,
	CR_INTERCEPT_CR9_READ = 1 << 9,
	CR_INTERCEPT_CR10_READ = 1 << 10,
	CR_INTERCEPT_CR11_READ = 1 << 11,
	CR_INTERCEPT_CR12_READ = 1 << 12,
	CR_INTERCEPT_CR13_READ = 1 << 13,
	CR_INTERCEPT_CR14_READ = 1 << 14,
	CR_INTERCEPT_CR15_READ = 1 << 15,
	CR_INTERCEPT_CR0_WRITE = 1 << 16,
	CR_INTERCEPT_CR1_WRITE = 1 << 17,
	CR_INTERCEPT_CR2_WRITE = 1 << 18,
	CR_INTERCEPT_CR3_WRITE = 1 << 19,
	CR_INTERCEPT_CR4_WRITE = 1 << 20,
	CR_INTERCEPT_CR5_WRITE = 1 << 21,
	CR_INTERCEPT_CR6_WRITE = 1 << 22,
	CR_INTERCEPT_CR7_WRITE = 1 << 23,
	CR_INTERCEPT_CR8_WRITE = 1 << 24,
	CR_INTERCEPT_CR9_WRITE = 1 << 25,
	CR_INTERCEPT_CR10_WRITE = 1 << 26,
	CR_INTERCEPT_CR11_WRITE = 1 << 27,
	CR_INTERCEPT_CR12_WRITE = 1 << 28,
	CR_INTERCEPT_CR13_WRITE = 1 << 29,
	CR_INTERCEPT_CR14_WRITE = 1 << 30,
	CR_INTERCEPT_CR15_WRITE = 1 << 31
};

enum DRInterceptBits
{
	DR_INTERCEPT_DR0_READ   = 1 << 0,
	DR_INTERCEPT_DR1_READ   = 1 << 1,
	DR_INTERCEPT_DR2_READ   = 1 << 2,
	DR_INTERCEPT_DR3_READ   = 1 << 3,
	DR_INTERCEPT_DR4_READ   = 1 << 4,
	DR_INTERCEPT_DR5_READ   = 1 << 5,
	DR_INTERCEPT_DR6_READ   = 1 << 6,
	DR_INTERCEPT_DR7_READ   = 1 << 7,
	DR_INTERCEPT_DR8_READ   = 1 << 8,
	DR_INTERCEPT_DR9_READ   = 1 << 9,
	DR_INTERCEPT_DR10_READ  = 1 << 10,
	DR_INTERCEPT_DR11_READ  = 1 << 11,
	DR_INTERCEPT_DR12_READ  = 1 << 12,
	DR_INTERCEPT_DR13_READ  = 1 << 13,
	DR_INTERCEPT_DR14_READ  = 1 << 14,
	DR_INTERCEPT_DR15_READ  = 1 << 15,
	DR_INTERCEPT_DR0_WRITE  = 1 << 16,
	DR_INTERCEPT_DR1_WRITE  = 1 << 17,
	DR_INTERCEPT_DR2_WRITE  = 1 << 18,
	DR_INTERCEPT_DR3_WRITE  = 1 << 19,
	DR_INTERCEPT_DR4_WRITE  = 1 << 20,
	DR_INTERCEPT_DR5_WRITE  = 1 << 21,
	DR_INTERCEPT_DR6_WRITE  = 1 << 22,
	DR_INTERCEPT_DR7_WRITE  = 1 << 23,
	DR_INTERCEPT_DR8_WRITE  = 1 << 24,
	DR_INTERCEPT_DR9_WRITE  = 1 << 25,
	DR_INTERCEPT_DR10_WRITE = 1 << 26,
	DR_INTERCEPT_DR11_WRITE = 1 << 27,
	DR_INTERCEPT_DR12_WRITE = 1 << 28,
	DR_INTERCEPT_DR13_WRITE = 1 << 29,
	DR_INTERCEPT_DR14_WRITE = 1 << 30,
	DR_INTERCEPT_DR15_WRITE = 1 << 31,
};

enum VMEXIT_EXITCODE
{
	/* control register read exitcodes */
	VMEXIT_CR0_READ    =   0,
	VMEXIT_CR1_READ    =   1,
	VMEXIT_CR2_READ    =   2,
	VMEXIT_CR3_READ    =   3,
	VMEXIT_CR4_READ    =   4,
	VMEXIT_CR5_READ    =   5,
	VMEXIT_CR6_READ    =   6,
	VMEXIT_CR7_READ    =   7,
	VMEXIT_CR8_READ    =   8,
	VMEXIT_CR9_READ    =   9,
	VMEXIT_CR10_READ   =  10,
	VMEXIT_CR11_READ   =  11,
	VMEXIT_CR12_READ   =  12,
	VMEXIT_CR13_READ   =  13,
	VMEXIT_CR14_READ   =  14,
	VMEXIT_CR15_READ   =  15,
	
	/* control register write exitcodes */
	VMEXIT_CR0_WRITE   =  16,
	VMEXIT_CR1_WRITE   =  17,
	VMEXIT_CR2_WRITE   =  18,
	VMEXIT_CR3_WRITE   =  19,
	VMEXIT_CR4_WRITE   =  20,
	VMEXIT_CR5_WRITE   =  21,
	VMEXIT_CR6_WRITE   =  22,
	VMEXIT_CR7_WRITE   =  23,
	VMEXIT_CR8_WRITE   =  24,
	VMEXIT_CR9_WRITE   =  25,
	VMEXIT_CR10_WRITE  =  26,
	VMEXIT_CR11_WRITE  =  27,
	VMEXIT_CR12_WRITE  =  28,
	VMEXIT_CR13_WRITE  =  29,
	VMEXIT_CR14_WRITE  =  30,
	VMEXIT_CR15_WRITE  =  31,
	
	/* debug register read exitcodes */
	VMEXIT_DR0_READ    =  32,
	VMEXIT_DR1_READ    =  33,
	VMEXIT_DR2_READ    =  34,
	VMEXIT_DR3_READ    =  35,
	VMEXIT_DR4_READ    =  36,
	VMEXIT_DR5_READ    =  37,
	VMEXIT_DR6_READ    =  38,
	VMEXIT_DR7_READ    =  39,
	VMEXIT_DR8_READ    =  40,
	VMEXIT_DR9_READ    =  41,
	VMEXIT_DR10_READ   =  42,
	VMEXIT_DR11_READ   =  43,
	VMEXIT_DR12_READ   =  44,
	VMEXIT_DR13_READ   =  45,
	VMEXIT_DR14_READ   =  46,
	VMEXIT_DR15_READ   =  47,
	
	/* debug register write exitcodes */
	VMEXIT_DR0_WRITE   =  48,
	VMEXIT_DR1_WRITE   =  49,
	VMEXIT_DR2_WRITE   =  50,
	VMEXIT_DR3_WRITE   =  51,
	VMEXIT_DR4_WRITE   =  52,
	VMEXIT_DR5_WRITE   =  53,
	VMEXIT_DR6_WRITE   =  54,
	VMEXIT_DR7_WRITE   =  55,
	VMEXIT_DR8_WRITE   =  56,
	VMEXIT_DR9_WRITE   =  57,
	VMEXIT_DR10_WRITE  =  58,
	VMEXIT_DR11_WRITE  =  59,
	VMEXIT_DR12_WRITE  =  60,
	VMEXIT_DR13_WRITE  =  61,
	VMEXIT_DR14_WRITE  =  62,
	VMEXIT_DR15_WRITE  =  63,
	
	/* processor exception exitcodes (VMEXIT_EXCP[0-31]) */
	VMEXIT_EXCEPTION_DE  =  64, /* divide-by-zero-error */
	VMEXIT_EXCEPTION_DB  =  65, /* debug */
	VMEXIT_EXCEPTION_NMI =  66, /* non-maskable-interrupt */
	VMEXIT_EXCEPTION_BP  =  67, /* breakpoint */
	VMEXIT_EXCEPTION_OF  =  68, /* overflow */
	VMEXIT_EXCEPTION_BR  =  69, /* bound-range */
	VMEXIT_EXCEPTION_UD  =  70, /* invalid-opcode*/
	VMEXIT_EXCEPTION_NM  =  71, /* device-not-available */
	VMEXIT_EXCEPTION_DF  =  72, /* double-fault */
	VMEXIT_EXCEPTION_09  =  73, /* unsupported (reserved) */
	VMEXIT_EXCEPTION_TS  =  74, /* invalid-tss */
	VMEXIT_EXCEPTION_NP  =  75, /* segment-not-present */
	VMEXIT_EXCEPTION_SS  =  76, /* stack */
	VMEXIT_EXCEPTION_GP  =  77, /* general-protection */
	VMEXIT_EXCEPTION_PF  =  78, /* page-fault */
	VMEXIT_EXCEPTION_15  =  79, /* reserved */
	VMEXIT_EXCEPTION_MF  =  80, /* x87 floating-point exception-pending */
	VMEXIT_EXCEPTION_AC  =  81, /* alignment-check */
	VMEXIT_EXCEPTION_MC  =  82, /* machine-check */
	VMEXIT_EXCEPTION_XF  =  83, /* simd floating-point */
	
	/* exceptions 20-31 (exitcodes 84-95) are reserved */
	
    /* ...and the rest of the #VMEXITs */
	VMEXIT_INTR             =  96,
	VMEXIT_NMI              =  97,
	VMEXIT_SMI              =  98,
	VMEXIT_INIT             =  99,
	VMEXIT_VINTR            = 100,
	VMEXIT_CR0_SEL_WRITE    = 101,
	VMEXIT_IDTR_READ        = 102,
	VMEXIT_GDTR_READ        = 103,
	VMEXIT_LDTR_READ        = 104,
	VMEXIT_TR_READ          = 105,
	VMEXIT_IDTR_WRITE       = 106,
	VMEXIT_GDTR_WRITE       = 107,
	VMEXIT_LDTR_WRITE       = 108,
	VMEXIT_TR_WRITE         = 109,
	VMEXIT_RDTSC            = 110,
	VMEXIT_RDPMC            = 111,
	VMEXIT_PUSHF            = 112,
	VMEXIT_POPF             = 113,
	VMEXIT_CPUID            = 114,
	VMEXIT_RSM              = 115,
	VMEXIT_IRET             = 116,
	VMEXIT_SWINT            = 117,
	VMEXIT_INVD             = 118,
	VMEXIT_PAUSE            = 119,
	VMEXIT_HLT              = 120,
	VMEXIT_INVLPG           = 121,
	VMEXIT_INVLPGA          = 122,
	VMEXIT_IOIO             = 123,
	VMEXIT_MSR              = 124,
	VMEXIT_TASK_SWITCH      = 125,
	VMEXIT_FERR_FREEZE      = 126,
	VMEXIT_SHUTDOWN         = 127,
	VMEXIT_VMRUN            = 128,
	VMEXIT_VMMCALL          = 129,
	VMEXIT_VMLOAD           = 130,
	VMEXIT_VMSAVE           = 131,
	VMEXIT_STGI             = 132,
	VMEXIT_CLGI             = 133,
	VMEXIT_SKINIT           = 134,
	VMEXIT_RDTSCP           = 135,
	VMEXIT_ICEBP            = 136,
	VMEXIT_WBINVD           = 137,
	VMEXIT_MONITOR          = 138,
	VMEXIT_MWAIT            = 139,
	VMEXIT_MWAIT_CONDITIONAL= 140,
	VMEXIT_NPF              = 1024, /* nested paging fault */
	VMEXIT_INVALID          =  -1
};

enum seg_atrr
{
	SEG_P  = 1<<2,
	SEG_W  = 1<<3,
	SEG_R  = 1<<4,
	SEG_X  = 1<<5,
};


struct svm_segment
{
	uint16_t sel;
	uint16_t attrib;
	uint32_t limit;
	uint64_t base;
}__attribute__((packed));

struct vintr_t
{
	uint64_t tpr: 8;
	uint64_t irq: 1;
	uint64_t rsvd0: 7;
	uint64_t prio: 4;
	uint64_t ign_tpr: 1;
	uint64_t rsvd1: 3;
	uint64_t intr_masking: 1;
	uint64_t rsvd2: 7;
	uint64_t vector: 8;
	uint64_t rsvd3: 24;
}__attribute__((packed));

struct ioio_info_t
{
	uint64_t type :1;
	uint64_t rsvd0 :1;
	uint64_t str :1;
	uint64_t rep :1;
	uint64_t sz8 :1;
	uint64_t sz16 :1;
	uint64_t sz32 :1;
	uint64_t rsv1 :9;
	uint64_t port :16;
}__attribute__((packed));

struct eveninj_t
{
	uint64_t vector :8;
	uint64_t type :3;
	uint64_t ev :1;
	uint64_t rsvd :19;
	uint64_t v :1;
	uint64_t errorcode :32;
}__attribute__((packed));


struct vmcb
{
	/*vmcb control area*/
	uint32_t cr_intercepts;
	uint32_t dr_intercepts;
	uint32_t exc_intercepts;
	uint32_t intercepts1;
	uint32_t intercepts2;
	uint32_t resv1[11];
	uint64_t iopm_base_pa;
	uint64_t msrpm_base_pa;
	uint64_t tsc_offset;
	uint32_t guest_asid;
	uint8_t  tlb_control;
	uint8_t resv2[3];
	struct vintr_t intr_t;
	uint64_t intr_shadow;
	uint64_t exitcode;
	uint64_t exitinfo1;
	uint64_t exitinfo2;
	struct eveninj_t exitinfo;
	uint64_t np_enable;
	uint64_t resv3[2];
	struct eveninj_t event_inj;
	uint64_t n_cr3;
	uint64_t lbr_virtual;
	uint64_t resv4[104];


	/*vmcb saved area*/
	struct svm_segment es;
	struct svm_segment cs;
	struct svm_segment ss;
	struct svm_segment ds;
	struct svm_segment fs;
	struct svm_segment gs;
	struct svm_segment gdtr;
	struct svm_segment ldtr;
	struct svm_segment idtr;
	struct svm_segment tr;

	uint64_t resv5[5];
	uint8_t rseved[3];
	uint8_t cpl;
	uint32_t resv6;
	uint64_t efer;
	uint64_t resv7[14];
	uint64_t cr4;
	uint64_t cr3;
	uint64_t cr0;
	uint64_t dr7;
	uint64_t dr6;
	uint64_t rflags;
	uint64_t rip;
	uint64_t resv8[11];
	uint64_t rsp;
	uint64_t resv9[3];
	uint64_t rax;
	uint64_t star;
	uint64_t lstar;
	uint64_t cstar;
	uint64_t sfmask;
	uint64_t kernelgsbase;
	uint64_t sysenter_cs;
	uint64_t sysenter_esp;
	uint64_t sysenter_eip;
	uint64_t cr2;
	uint64_t resv10[4];
	uint64_t g_pat;
	uint64_t dbgctr;
	uint64_t br_from;
	uint64_t br_to;
	uint64_t lastexcpfrom;
	uint64_t lastexcpto;
	uint64_t resv11[301];
}__attribute__ ((__packed__));

struct vmcb * alloc_vmcb(void);

void freevmcb(struct vmcb *vmcb);
#endif
