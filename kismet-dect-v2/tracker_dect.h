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

#ifndef __TRACKER_DECT_H__
#define __TRACKER_DECT_H__

#include "config.h"

#include <packet.h>
#include <gpscore.h>

#include "packet_dect.h"

// Dect basestation
class dect_tracked_fp {
public:
	dect_tracked_fp() {
		first_time = last_time = 0;
		last_rssi = 0;
		num_seen = 0;
		channel = 0;
		dirty = 0;
	}

	mac_addr rfpi;
	time_t first_time, last_time;
	unsigned int last_rssi;
	unsigned int num_seen;
	unsigned int channel;
	unsigned int dirty;

	kis_gps_data gpsdata;
};

class Tracker_Dect {
public:
	Tracker_Dect() { fprintf(stderr, "FATAL OOPS: tracker_dect()\n"); exit(1); }
	Tracker_Dect(GlobalRegistry *in_globalreg);

	int chain_handler(kis_packet *in_pack);

	void BlitFP(int in_fd);

protected:
	GlobalRegistry *globalreg;

	map<mac_addr, dect_tracked_fp *> tracked_fp;

	int DECTFP_ref;
	int timer_ref;

};

#endif

