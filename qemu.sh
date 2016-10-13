#!/bin/sh
qemu-system-x86_64 \
	-hda ./obj/kern/bochs.img \
	-smp 4 \
	-m 256 \
	-nographic	
