#define __NR_read 0
__VMCALL(__NR_read, vm_read)

#define __NR_write 1
__VMCALL(__NR_write, vm_write)

#define __NR_open 2
__VMCALL(__NR_open, vm_open)

#define __NR_close 3
__VMCALL(__NR_close, vm_close)

#define __NR_vmmcons 4
__VMCALL(__NR_vmmcons, vmmcons_init)

#define __NR_vmstart 5
__VMCALL(__NR_vmstart, start_vm)

#define __NR_vmsocket 6

__VMCALL(__NR_vmsocket, vmm_sock)

#define __NR_vmapic 7
__VMCALL(__NR_vmapic, vmapic)

#define __NR_snull 8
__VMCALL(__NR_snull, snull_init)

#define __NR_tx 9
__VMCALL(__NR_tx, snull_tx)

#define __NR_irq0 10
__VMCALL(__NR_irq0, irq0_forward)

#define __NR_get 11
__VMCALL(__NR_get, snull_get)

#define __NR_get_dom_bid 12
__VMCALL(__NR_get_dom_bid, get_dom_bid)

#define __NR_domainid 13
__VMCALL(__NR_domainid, get_did)

#define __NR_guest_test 14
__VMCALL(__NR_guest_test, guest_test)

#define __NR_debug_init 17
__VMCALL(__NR_debug_init, debug_printf_init) 

//#define __NR_privacy_set 19
//__VMCALL(__NR_privacy_set, privacy_set)

//#define __NR_vmmcall_test 21
//__VMCALL(__NR_vmmcall_test, vmmcall_test)

//#define __NR_vmmcall_domprint 22
//__VMCALL(__NR_vmmcall_domprint, vmmcall_domprint)

#define __NR_vmmcall_intr_des 23
__VMCALL(__NR_vmmcall_intr_des, vmmcall_intr_des)

//#define __NR_vmmcall_step_enable 24
//__VMCALL(__NR_vmmcall_step_enable, vmmcall_step_enable)

