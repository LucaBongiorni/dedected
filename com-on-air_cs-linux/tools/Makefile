CFLAGS=-Wall -O2 -I..
PROGS=coa_syncsniff pcap2cchan
PCAP_PROGS=pcapstein dect_cli
all:$(PROGS) $(PCAP_PROGS)

$(PCAP_PROGS): $(foreach p,$(PCAP_PROGS), $p.c)
	$(CC) $(CFLAGS) -lpcap $@.c -o $@
clean:
	rm -f $(PROGS) $(PCAP_PROGS)
