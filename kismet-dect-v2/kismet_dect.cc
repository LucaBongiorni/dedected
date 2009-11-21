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

#include <config.h>
#include <string>
#include <errno.h>
#include <time.h>

#include <pthread.h>

#include <sstream>
#include <iomanip>

#include <util.h>
#include <messagebus.h>
#include <packet.h>
#include <packetchain.h>
#include <packetsource.h>
#include <packetsourcetracker.h>
#include <timetracker.h>
#include <configfile.h>
#include <plugintracker.h>
#include <globalregistry.h>
#include <netracker.h>
#include <packetdissectors.h>
#include <alertracker.h>
#include <dumpfile_pcap.h>
#include <version.h>

#include "packetsource_dect.h"
#include "tracker_dect.h"

GlobalRegistry *globalreg = NULL;

int pack_comp_dect;

kis_datachunk *dumpfile_dect_filter(DUMPFILE_PCAP_FILTER_PARMS) {
	kis_datachunk *link = (kis_datachunk *) in_pack->fetch(_PCM(PACK_COMP_LINKFRAME));

	if (link == NULL)
		return NULL;

	if (link->dlt == KDLT_DECT)
		return link;

	return NULL;
}

int cmd_SETDECTMODE(CLIENT_PARMS) {
	if (parsedcmdline->size() < 2) {
		snprintf(errstr, 1024, "Illegal SETDECTMODE command, expected UUID, mode");
		return -1;
	}

	uuid inuuid = uuid((*parsedcmdline)[0].word);

	if (inuuid.error) {
		snprintf(errstr, 1024, "Invalid UUID in SETDECTMODE command");
		return -1;
	}

	pst_packetsource *pstsource = 
		globalreg->sourcetracker->FindLivePacketSourceUUID(inuuid);

	if (pstsource == NULL || pstsource->strong_source == NULL) {
		snprintf(errstr, 1024, "Invalid UUID in SETDECTMODE command, couldn't find "
				 "source with UUID %s", inuuid.UUID2String().c_str());
		return -1;
	}

	if (pstsource->strong_source->FetchType() != "dect" &&
		pstsource->strong_source->FetchType() != "dectus") {
		snprintf(errstr, 1024, "Invalid source in SETDECTMODE, source with UUID %s "
				 "doesn't look like a DECT device", inuuid.UUID2String().c_str());
		return -1;
	}

	string mode = StrLower((*parsedcmdline)[1].word);

	if (mode == "fpscan") {
		if (((PacketSource_Dect *) pstsource->strong_source)->StartScanFp() < 0) {
			snprintf(errstr, 1024, "DECT source failed to start FP scan mode on "
					 "source %s", inuuid.UUID2String().c_str());
			return -1;
		} 

		if (globalreg->sourcetracker->SetSourceHopping(inuuid, 1, 0) < 0) {
			snprintf(errstr, 1024, "DECT source failed to turn on hopping for FP "
					 "scan mode on source %s", inuuid.UUID2String().c_str());
			return -1;
		}

		return 1;
	} else if (mode == "ppscan") {
		if (parsedcmdline->size() < 4) {
			snprintf(errstr, 1024, "Illegal SETDECTMODE command, expected UUID "
					 "ppscan rfpi channel");
			return -1;
		}

		// Hack into a macaddr
		mac_addr inrfpi = mac_addr(string((*parsedcmdline)[2].word + ":00"));

		if (inrfpi.error) {
			snprintf(errstr, 1024, "Invalid RFPI in SETDECTMODE ppscan command");
			return -1;
		}

		unsigned int inchan;
		if (sscanf((*parsedcmdline)[3].word.c_str(), "%u", &inchan) != 1) {
			snprintf(errstr, 1024, "Invalid channel in SETDECTMODE ppscan command");
		}

		if (((PacketSource_Dect *) pstsource->strong_source)->StartScanPp(inrfpi) < 0) {
			snprintf(errstr, 1024, "DECT source failed to start PP scan mode on "
					 "source %s", inuuid.UUID2String().c_str());
			return -1;
		} 

		if (globalreg->sourcetracker->SetSourceHopping(inuuid, 0, inchan) < 0) {
			snprintf(errstr, 1024, "DECT source failed to turn off hopping for PP "
					 "scan mode on source %s", inuuid.UUID2String().c_str());
			return -1;
		}

		return 1;
	}
}

int dect_unregister(GlobalRegistry *in_globalreg) {
	return 0;
}

int dect_register(GlobalRegistry *in_globalreg) {
	globalreg = in_globalreg;

	globalreg->sourcetracker->AddChannelList("DECT:0,1,2,3,4,5,6,7,8,9");
	globalreg->sourcetracker->AddChannelList("DECTUS:0,1,2,3,4,5,6,7,8,9,23,24,25,26,27");
	
	if (globalreg->sourcetracker->RegisterPacketSource(new PacketSource_Dect(globalreg)) < 0 || globalreg->fatal_condition) {
		return -1;
	}

	pack_comp_dect =
		globalreg->packetchain->RegisterPacketComponent("DECT");

	// dumpfile that inherits from the global one, with a hack in the matcher to
	// pick the dect packets out but log them as EN10MB
	Dumpfile_Pcap *dectdump;
	dectdump = 
		new Dumpfile_Pcap(globalreg, "pcapdect", DLT_EN10MB,
						  globalreg->pcapdump, dumpfile_dect_filter, NULL);
	dectdump->SetVolatile(1);

	Tracker_Dect *trackdect;
	trackdect = new Tracker_Dect(globalreg);

	return 1;
}

extern "C" {
	int kis_plugin_info(plugin_usrdata *data) {
		data->pl_name = "DECT";
		data->pl_version = "2.0.0";
		data->pl_description = "deDECTed com-on-air_cs plugin";
		data->pl_unloadable = 0; // We can't be unloaded because we defined a source
		data->plugin_register = dect_register;
		data->plugin_unregister = dect_unregister;

		return 1;
	}

	void kis_revision_info(plugin_revision *prev) {
		if (prev->version_api_revision >= 1) {
			prev->version_api_revision = 1;
			prev->major = string(VERSION_MAJOR);
			prev->minor = string(VERSION_MINOR);
			prev->tiny = string(VERSION_TINY);
		}
	}
}


