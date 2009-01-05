/*
 * pcapstein dumps all B-Fields found in a pcap file
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * authors:
 * (C) 2008  Andreas Schuler <krater at badterrorist dot com>
 * (C) 2008  Matthias Wenzel <dect at mazzoo dot de>
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <pcap.h>

#include "pcapstein.h"

struct file_info fi;

int pp_slot = -1;
int fp_slot = -1;

void usage(void)
{
	fprintf(stderr, "usage: pcapstein <dect-pcap-file>\n");
	fprintf(stderr, "       creates <dect-pcap-file>_pp.ima\n");
	fprintf(stderr, "       and     <dect-pcap-file>_fp.ima\n");
	fprintf(stderr, "       for further g.721 audio processing\n");
	fprintf(stderr, "       e.g. decode and sox\n");
}

void init(char * fname)
{
	fi.p = pcap_open_offline(fname, errbuf);
	if (!fi.p)
	{
		fprintf(stderr, "couln't open(\"%s\"): %s\n",
			fname,
			strerror(errno));
		fprintf(stderr, "pcap error: %s\n",
			errbuf);
		exit(1);
	}
	fprintf(stderr, "%s\n",
		pcap_lib_version());
	fprintf(stderr, "pcap file version %d.%d\n",
		pcap_major_version(fi.p),
		pcap_minor_version(fi.p)
		);

	/* output file handles */
	char * imafname = malloc(strlen(fname)+23);
	if (!imafname)
	{
		fprintf(stderr, "out of memory: %s\n",
			strerror(errno));
		exit(1);
	}

	/* PP output file */
	sprintf(imafname, "%s_pp.ima", fname);
	fi.fpp = open(imafname, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
	if (!fi.fpp)
	{
		fprintf(stderr, "couldn't open(\"%s\"): %s\n",
			imafname,
			strerror(errno));
		exit(1);
	}

	/* FP output file */
	sprintf(imafname, "%s_fp.ima", fname);
	fi.ffp = open(imafname, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
	if (!fi.ffp)
	{
		fprintf(stderr, "couldn't open(\"%s\"): %s\n",
			imafname,
			strerror(errno));
		exit(1);
	}

	free (imafname);
}

void swap_nibbles(u_char * buf, int len)
{
	int i;
	u_char v;
	for (i = 0; i < len; i++) {
		v = buf[i];
		buf[i] = ((v>>4)&15)|((v&15)<<4);
	}
}

void process_b_field(const struct pcap_pkthdr *h, const u_char *pkt)
{
	u_char buf[40];

	if ( (pkt[0x17] == 0x16) && (pkt[0x18] == 0x75) )
	{
		if (pp_slot < 0)
		{
			pp_slot = pkt[0x11];
		}else{
			if (pp_slot == pkt[0x11])
			{
				memcpy(buf, &pkt[0x21], 40);
				swap_nibbles(buf, 40);
				write(fi.fpp, buf, 40);
			}
		}
	}
	else
	{
		if (fp_slot < 0)
		{
			fp_slot = pkt[0x11];
		}else{
			if (fp_slot == pkt[0x11])
			{
				memcpy(buf, &pkt[0x21], 40);
				swap_nibbles(buf, 40);
				write(fi.ffp, buf, 40);
			}
		}
	}
}

void process_pcap_packet(
	u_char *user, const struct pcap_pkthdr *h,
	const u_char *pkt)
{
#if 0
	int i;
	printf("packet for %s [%u/%u] @%d.%d\n",
		user,
		h->len,
		h->caplen,
		h->ts.tv_sec,
		h->ts.tv_usec
		);
#endif

	if (pkt[ETH_TYPE_0_OFF] != ETH_TYPE_0)
		return;
	if (pkt[ETH_TYPE_1_OFF] != ETH_TYPE_1)
		return;
#if 0
	for (i=0; i<sizeof(dect_pkt_hdr); i++)
		printf("%.2x ", pkt[i]);
	printf("\n");
#endif

	if ((pkt[PKT_OFF_H] & DECT_H_BA_MASK) == DECT_H_BA_NO_B_FIELD)
		return;

	if ((pkt[PKT_OFF_H] & DECT_H_BA_MASK) == DECT_H_BA_HALF_SLOT)
	{
		fprintf(stderr, "unsopported half slot\n");
		return;
	}

	if ((pkt[PKT_OFF_H] & DECT_H_BA_MASK) == DECT_H_BA_DOUBLE_SLOT)
	{
		fprintf(stderr, "unsopported double slot\n");
		return;
	}
	process_b_field(h, pkt);
#if 0
	for (i=0; i<23; i++)
		printf("%.2x ", pkt[i]);
	printf("\n");
#endif
}

void play()
{
	int ret;
	ret = pcap_loop(
		fi.p,
		-1, /* forever */
		process_pcap_packet,
		NULL);
	fprintf(stderr, "pcap_loop() = %d\n", ret);
	if (*errbuf)
		fprintf(stderr, "pcap error: %s\n", errbuf);
}

void shutdown()
{
	pcap_close(fi.p);
	close(fi.fpp);
	close(fi.ffp);
}

int main(int argc, char ** argv)
{
	if (argc != 2)
	{
		usage();
		exit(1);
	}
	init(argv[1]);
	play();
	shutdown();
	return 0;
}
