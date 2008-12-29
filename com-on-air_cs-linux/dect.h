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

#ifndef DECT_H
#define DECT_H

#include <linux/string.h>
#include <linux/kernel.h>

#define DECT_SLOTTYPE_CARRIER	0
#define DECT_SLOTTYPE_SCAN	1

struct dect_slot_info
{
	uint8_t         active;
	uint8_t         channel;
	uint8_t         type;
	uint8_t		errcnt;
	uint8_t		update;
};


int dect_is_RFPI_Packet(unsigned char *packet);
int dect_compare_RFPI(unsigned char *packet, unsigned char* RFPI);
int dect_get_slot(unsigned char *packet);
int dect_is_multiframe_number(unsigned char *packet);
int dect_is_fp_packet(unsigned char *packet);
int dect_is_pp_packet(unsigned char *packet);
int dect_update_slottable(struct dect_slot_info *slottable, int slot, unsigned char *packet);
int dect_receive_error(struct dect_slot_info *slottable, int slot);
int dect_update_scanchannels(struct dect_slot_info *slottable);
#endif
