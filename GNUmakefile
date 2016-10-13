OBJDIR := obj




OBJDIRS :=
V :=
TOP :=.
CFLAGS := $(CFLAGS) -O1 -fno-builtin -isystem $(shell gcc -print-file-name=include) -I$(TOP)
CFLAGS += -Wall -Wno-format -Os -Wno-unused -Werror -gstabs -fno-stack-protector
CFLAGS += -std=c99 
LDFLAGS := -m elf_x86_64

KERN_CFLAGS := $(CFLAGS) -DJOS_KERNEL -gstabs
PREFIX :=


CC := $(PREFIX)gcc -pipe
AS := $(PREFIX)as
AR := $(PREFIX)ar
LD := $(PREFIX)ld
OBJCOPY := $(PREFIX)objcopy
OBJDUMP := $(PREFIX)objdump
NM := $(PREFIX)nm

all:

include boot/Makefrag
include kern/Makefrag

clean:
	rm -rf obj

