CFLAGS=-Wall -O2 -I..
PROGS=coa_syncsniff pcap2cchan dump_eeprom dump_dip
PCAP_PROGS=pcapstein dect_cli
all:$(PROGS) $(PCAP_PROGS)

$(PCAP_PROGS): $(foreach p,$(PCAP_PROGS), $p.c)
	$(CC) $(CFLAGS) $@.c -o $@ -lpcap
clean:
	rm -f $(PROGS) $(PCAP_PROGS)
