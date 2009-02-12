/*
 * com_on_air_cs - basic driver for the Dosch and Amand "com on air" cards
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

#ifndef SC14421_SNIFF_H
#define SC14421_SNIFF_H

#include "com_on_air_user.h"

#define SNIFF_SCANFP	COA_SUBMODE_SNIFF_SCANFP
#define SNIFF_SCANPP	COA_SUBMODE_SNIFF_SCANPP
#define SNIFF_SYNC	COA_SUBMODE_SNIFF_SYNC

#define SNIFF_STATUS_FOUNDSTATION	0x80
#define SNIFF_STATUS_INSYNC		0x40

#define SNIFF_SLOTPATCH_FP	0
#define SNIFF_SLOTPATCH_PP	1

#include "dect.h"
#include "com_on_air.h"

struct sniffer_cfg
{
	int			snifftype;
	int			channel;
	unsigned char		RFPI[5];
	unsigned char		status;
	struct dect_slot_info	slottable[24];
	int			framenumber;
	int			updatefpslots;
	int			updateppslots;
};

struct sniffed_rfpi
{
	unsigned char rssi;
	unsigned char channel;
	unsigned char RFPI[5];
};

#define SLOT_OUT_OF_SYNC  0x20
#define SLOT_IN_SYNC      0x21

struct sniffed_packet
{
	unsigned char     rssi;
	unsigned char     channel;
	unsigned char     slot;
	unsigned char     framenumber;
//	unsigned char     bfok;
	struct timespec   timestamp;
	unsigned char     data[53];
};


void    sniffer_init(struct coa_info *dev);
void    sniffer_init_sniff_all(struct coa_info *dev);
void    sniffer_init_sniff_scan(struct coa_info *dev);
void    sniffer_init_sniff_sync(struct coa_info *dev);
uint8_t sniffer_irq_handler(struct coa_info *dev);
void    sniffer_sniff_all_irq(struct coa_info *dev,int irq);
void    sniffer_sniff_scan_irq(struct coa_info *dev,int irq);
void    sniffer_sniff_sync_irq(struct coa_info *dev,int irq);
void    sniffer_sync_patchloop(struct coa_info *dev,struct dect_slot_info *slottable,int type);
void    sniffer_clear_slottable(struct dect_slot_info *slottable);
#endif

