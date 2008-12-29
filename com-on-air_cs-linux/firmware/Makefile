CFLAGS=-Wall -O2

FW=sc14421_firmware

FW1=sc14421_II_sniff_scan
FW2=sc14421_II_sniff_sync

ALL_FW=$(FW1) $(FW2)

BIN2C=./bin2c

all: ../$(FW).h ../$(FW).c

../$(FW).h: $(FW1).h $(FW2).h
	cat sc14421_header.h $^ sc14421_footer.h > $@

../$(FW).c: $(FW1).c $(FW2).c
	cat $^ > $@

$(FW1).c $(FW2).c: $(FW1).bin $(FW2).bin $(BIN2C)
	$(BIN2C) $(@:.c=.bin) $(@:.c=)_fw > $@

%.bin: %.p
	p2bin $^ $@ -r 0-507

%.p: %.asm
	asl $^

$(FW1).h $(FW2).h:
	echo "extern unsigned char $(@:.h=)_fw[509];" > $@

clean:
	rm -f ../$(FW).h ../$(FW).c $(BIN2C)
	rm -f $(foreach f,$(ALL_FW),$f.c)
	rm -f $(foreach f,$(ALL_FW),$f.h)
	rm -f $(foreach f,$(ALL_FW),$f.bin)

