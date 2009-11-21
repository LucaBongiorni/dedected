/*
    This file is part of Kismet

    Kismet is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kismet is distributed in the hope that it will be useful,
      but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Kismet; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

	DECT source v2
	(c) 2008 by Mike Kershaw <dragorn (at) kismetwireless (dot) net,
		    Jacob Appelbaum <jacob (at) appelbaum (dot) net,
		    Christian Fromme <kaner (at) strace (dot) org
*/

#ifndef __PACKET_DECT_H__
#define __PACKET_DECT_H__

#include "config.h"

#include <packet.h>

// Internal DLT - -0xDEC
#define KDLT_DECT		-3564

extern int pack_comp_dect;

/* This is the 7 bytes we read while scanning */
typedef struct {
	uint8_t               channel;
	uint8_t               RSSI;
	uint8_t               RFPI[5];
} dect_data_scan_t;

typedef struct {
	unsigned char rssi;
	unsigned char channel;
	unsigned char slot;
	struct timespec   timestamp;
	unsigned char data[53];
} pp_packet_t;

class dect_packinfo : public packet_component {
public:
	dect_packinfo() {
		self_destruct = 1;
		memset(&sdata, 0, sizeof(dect_data_scan_t));
		memset(&pdata, 0, sizeof(pp_packet_t));
		scanmode = -1;
		sync = false;
		channel = 0;
	}

	dect_data_scan_t sdata;
	pp_packet_t pdata;
	int scanmode;
	bool sync;
	uint16_t channel;
};

#endif

