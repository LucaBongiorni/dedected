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

#include "config.h"

#include <globalregistry.h>
#include <packetchain.h>
#include <util.h>

#include "packet_dect.h"
#include "tracker_dect.h"
#include "packetsource_dect.h"

enum DECTFP_fields {
	DECTFP_rfpi, DECTFP_firsttime, DECTFP_lasttime, DECTFP_lastrssi,
	DECTFP_num, DECTFP_channel, 
	DECTFP_maxfield
};

const char *DECTFP_fields_text[] = {
	"rfpi", "firsttime", "lasttime", "lastrssi",
	"num", "channel",
	NULL
};

int Protocol_DECTFP(PROTO_PARMS) {
	dect_tracked_fp *fp = (dect_tracked_fp *) data;
	ostringstream osstr;

	cache->Filled(field_vec->size());

	for (unsigned int x = 0; x < field_vec->size(); x++) {
		unsigned int fnum = (*field_vec)[x];

		if (fnum >= DECTFP_maxfield) {
			out_string = "Unknown field requested.";
			return -1;
		}

		osstr.str("");

		if (cache->Filled(fnum)) {
			out_string += cache->GetCache(fnum) + " ";
			continue;
		}

		switch (fnum) {
			case DECTFP_rfpi:
				osstr << fp->rfpi.Mac2String().substr(0, 14);
				break;
			case DECTFP_firsttime:
				osstr << fp->first_time;
				break;
			case DECTFP_lasttime:
				osstr << fp->last_time;
				break;
			case DECTFP_lastrssi:
				osstr << fp->last_rssi;
				break;
			case DECTFP_num:
				osstr << fp->num_seen;
				break;
			case DECTFP_channel:
				osstr << fp->channel;
				break;
		}

		out_string += osstr.str() + " ";
		cache->Cache(fnum, osstr.str());
	}

	return 1;
}

void Protocol_DECTFP_enable(PROTO_ENABLE_PARMS) {
	((Tracker_Dect *) data)->BlitFP(in_fd);
}

int decttracktimer(TIMEEVENT_PARMS) {
	((Tracker_Dect *) parm)->BlitFP(-1);
	return 1;
}

int dect_chain_hook(CHAINCALL_PARMS) {
	return ((Tracker_Dect *) auxdata)->chain_handler(in_pack);
}

Tracker_Dect::Tracker_Dect(GlobalRegistry *in_globalreg) {
	globalreg = in_globalreg;

	globalreg->packetchain->RegisterHandler(&dect_chain_hook, this,
											CHAINPOS_CLASSIFIER, 0);

	DECTFP_ref =
		globalreg->kisnetserver->RegisterProtocol("DECTFP", 0, 1,
												  DECTFP_fields_text,
												  &Protocol_DECTFP,
												  &Protocol_DECTFP_enable,
												  this);

	timer_ref =
		globalreg->timetracker->RegisterTimer(SERVER_TIMESLICES_SEC, NULL, 1,
											  &decttracktimer, this);
}

int Tracker_Dect::chain_handler(kis_packet *in_pack) {
	dect_packinfo *di = (dect_packinfo *) in_pack->fetch(pack_comp_dect);
	dect_tracked_fp *fp = NULL;

	if (di == NULL)
		return 0;

	mac_addr rfpi_mac;
	uint8_t mod_mac[6];

	// If we're call data, don't process us (for now)
	if (di->scanmode == MODE_SYNC_CALL_SCAN) {
		return 0;
	}

	// If we have a FP or PP packet, get the rfpi as mac address
	if (di->scanmode == MODE_ASYNC_FP_SCAN ||
		di->scanmode == MODE_ASYNC_PP_SCAN) {
		memcpy(mod_mac, di->sdata.RFPI, 5);
		mod_mac[5] = 0;
		rfpi_mac = mac_addr(mod_mac);
	} else {
		return 0;
	}

	map<mac_addr, dect_tracked_fp *>::iterator dtfi = tracked_fp.find(rfpi_mac);

	if (dtfi == tracked_fp.end()) {
		fp = new dect_tracked_fp;

		fp->first_time = globalreg->timestamp.tv_sec;
		fp->channel = di->sdata.channel;
		fp->rfpi = rfpi_mac;
		tracked_fp[rfpi_mac] = fp;
		_MSG("debug - dect, new tracked FP " + rfpi_mac.Mac2String().substr(0, 14), MSGFLAG_INFO);
	} else {
		fp = dtfi->second;
	}

	if (fp->channel != di->sdata.channel) {
		_MSG("DECT Station " + rfpi_mac.Mac2String().substr(0, 14) + " changed channel "
			 "from " + IntToString(fp->channel) + " to " + 
			 IntToString(di->sdata.channel), MSGFLAG_INFO);
		fp->channel = di->sdata.channel;
	}

	fp->last_time = globalreg->timestamp.tv_sec;
	fp->num_seen++;
	fp->last_rssi = di->sdata.RSSI;
	fp->dirty = 1;

	return 0;
}


void Tracker_Dect::BlitFP(int in_fd) {
	map<mac_addr, dect_tracked_fp *>::iterator x;

	for (x = tracked_fp.begin(); x != tracked_fp.end(); x++) {
		kis_protocol_cache cache;

		if (in_fd == -1) {
			if (x->second->dirty == 0)
				continue;

			x->second->dirty = 0;

			if (globalreg->kisnetserver->SendToAll(DECTFP_ref,
												   (void *) x->second) < 0)
				break;
		} else {
			if (globalreg->kisnetserver->SendToClient(in_fd, DECTFP_ref,
													  (void *) x->second,
													  &cache) < 0)
				break;
		}
	}
}
