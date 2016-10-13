#include <inc/pci.h>

void pci_config_access(struct vmcb *vmcb)
{
	int did = read_pda(cpudid);
	uint32_t val_cf8;
	uint8_t type = vmcb->exitinfo1 & 1;

	val_cf8 = inl(0xcf8);

	if((val_cf8 & (1<<31)) == 0){	// Operate normal I/O port
normal_operates:
		switch((vmcb->exitinfo1>>4) & 7){
			case 1:
				if(type){
					vmcb->rax = inb(0xcfc);
				}else{
					outb(0xcfc, vmcb->rax);
				}
				break;
			case 2:
				if(type){
					vmcb->rax = inw(0xcfc);
				}else{
					outw(0xcfc, vmcb->rax);
				}
				break;
			case 4:
				if(type){
					vmcb->rax = inl(0xcfc);
				}else{
					outl(0xcfc, vmcb->rax);
				}
				break;
			default:
				while(1)
					lock_cprintf("Error operand size");
		}
		return;
	}

	if(did == 0){	// This is domain 0 access pci config address space
		if((val_cf8 & 0xfff800) != USB_KBD_PCI_ADDR){	// Operate on other pci device
			goto normal_operates;
		}else{	// Operate on USB KBD
			if(type){
				vmcb->rax = 0xffff;
			}
		}
	}else if(did == 1){	// Domain 1 access
		if((val_cf8 & 0xfff800) != USB_KBD_PCI_ADDR){	// Operate on other pci device
			if(type){
				vmcb->rax = 0xffff;
			}
		}else{	//Operate on USB KBD
			goto normal_operates;
		}
	}else{
		while(1)
			lock_cprintf("Wrong did\n");
	}
}

void mmio_copy(uint64_t source, uint64_t dest, int len)
{
	__asm__ __volatile__("1:movb (%%rbx), %%al; movb %%al, (%%rdx);"
								"inc %%rbx; inc %%rdx; loop 1b;"
								::"b"(source), "c"(len), "d"(dest)
								:"cc");

}

void get_pci_dw(uint32_t dev_id, int offset, uint32_t *val)
{
	uint32_t base_addr;
	base_addr = read_msr(0xc0010058);
	if((base_addr & 1) == 0){
		lock_cprintf("PCI configure space MSR access is disabled\n");
		*val = -1;
		return;
	}
	base_addr &= ~((1<<20)-1);
	base_addr += (dev_id << 12) + offset;
	mmio_copy(base_addr, (uint64_t)val, 4);
}

int get_pci_header(uint32_t dev_id, void *addr)
{
	uint32_t base_addr;
	base_addr = read_msr(0xc0010058);
	if((base_addr & 1) == 0){
		lock_cprintf("PCI configure space MSR access is disabled\n");
		return -1;
	}
	base_addr &= ~((1<<20)-1);
	base_addr += dev_id << 12;
	mmio_copy(base_addr, (uint64_t)addr, sizeof(struct pci_config_header));
	return 0;
}
