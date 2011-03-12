NODE:=/dev/coa

PCMCIA_SLOT?=0

-include pcmcia_slot.make

ifneq ($(KERNELRELEASE),)
	obj-m := com_on_air_cs.o
	com_on_air_cs-objs := com_on_air.o sc14421.o sc14421_sniffer.o sc14421_firmware.o dect.o
else
	KDIR ?= /lib/modules/`uname -r`/build/
	PWD  := $(shell pwd)

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

endif

load:	node
	insmod ./com_on_air_cs.ko && \
	pccardctl insert $(PCMCIA_SLOT)

unload:
	pccardctl eject $(PCMCIA_SLOT) && \
	rmmod com_on_air_cs

reload:
	pccardctl eject $(PCMCIA_SLOT) && \
	rmmod com_on_air_cs && \
	insmod ./com_on_air_cs.ko && \
	pccardctl insert $(PCMCIA_SLOT)

node: $(NODE)
$(NODE):
	mknod $@ --mode 660 c 3564 0  ###  3564 == 0xDEC
#	chgrp dect $(NODE)

read: node coa_read
	tools/coa_read

watch: node coa_read
	watch -n 0.2 ./tools/coa_read

clean:
	rm -rf com_on_air_cs.ko com_on_air_cs.mod.c .com_on_air* .sc14421* .tmp* Module.symvers modules.order *.o

