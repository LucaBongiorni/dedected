CFLAGS=-Wall -O2

FW=sc14421_firmware

FW1=sc14421_II_sniff_scan
FW2=sc14421_II_sniff_sync

ALL_FW=$(FW1) $(FW2)

BIN2C=./bin2c

all: ../$(FW).h ../$(FW).c

../$(FW).h: $(foreach f,$(ALL_FW),$f.decl $f.label)
	cat sc14421_header.h $^ sc14421_footer.h > $@

../$(FW).c: $(foreach f,$(ALL_FW),$f.c)
	cat $^ > $@

$(foreach f,$(ALL_FW),$f.c): $(foreach f,$(ALL_FW),$f.bin) $(BIN2C)
	$(BIN2C) $(@:.c=.bin) $(@:.c=)_fw > $@

%.bin: %.p
	p2bin $^ $@ -r 0-507

%.p: %.asm
	asl -c $^
	mv $(@:.p=.h) $(@:.p=).label

%.label:%.p
	@echo "labels in $@"

$(foreach f,$(ALL_FW),$f.decl):
	@echo "extern unsigned char $(@:.decl=)_fw[509];" > $@

clean:
	rm -f ../$(FW).h ../$(FW).c $(BIN2C)
	rm -f $(foreach f,$(ALL_FW),$f.c)
	rm -f $(foreach f,$(ALL_FW),$f.p)
	rm -f $(foreach f,$(ALL_FW),$f.decl)
	rm -f $(foreach f,$(ALL_FW),$f.label)
	rm -f $(foreach f,$(ALL_FW),$f.bin)

