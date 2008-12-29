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

#ifndef PCAPSTEIN_H
#define PCAPSTEIN_H

struct file_info
{
	pcap_t               * p;
	struct pcap_pkthdr   rec; /* walks through all packets */

	int                  fpp; /* filehandle to PP .ima file */
	int                  ffp; /* filehandle to FP .ima file */
};

char errbuf[PCAP_ERRBUF_SIZE];

#define ETH_TYPE_0_OFF         0x0c
#define ETH_TYPE_1_OFF         0x0d
#define ETH_TYPE_0             0x23
#define ETH_TYPE_1             0x23

#define PKT_OFF_H              0x19
#define PKT_OFF_TAIL           0x1a
#define PKT_OFF_R_CRC          0x1f
#define PKT_OFF_B_FIELD        0x21

#define DECT_H_BA_MASK         0x0e
#define DECT_H_BA_NO_B_FIELD   0x0e
#define DECT_H_BA_HALF_SLOT    0x08
#define DECT_H_BA_DOUBLE_SLOT  0x04


#endif /* PCAPSTEIN_H */
