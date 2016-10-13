#define fr_rax 0
#define fr_rbx 8
#define fr_rcx 16
#define fr_rdx 24
#define fr_rdi 32
#define fr_rsi 40
#define fr_rbp 48
#define fr_r8  56
#define fr_r9  64
#define fr_r10 72
#define fr_r11 80
#define fr_r12 88
#define fr_r13 96
#define fr_r14 104
#define fr_r15 112
#define err_code 120
#define orig_rip 128
#define orig_cs  136
#define orig_eflags 144
#define orig_rsp 152
#define orig_ss  160
#define ENTRY(name) .global name;\
	name:


