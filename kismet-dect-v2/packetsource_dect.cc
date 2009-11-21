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

#include <packetsource.h>
#include "packet_dect.h"
#include "packetsource_dect.h"

PacketSource_Dect::PacketSource_Dect(GlobalRegistry *in_globalreg):
	KisPacketSource(in_globalreg) {

	serial_fd = -1;
	sync = false;
	scan_mode = MODE_ASYNC_FP_SCAN;
}

PacketSource_Dect::PacketSource_Dect(GlobalRegistry *in_globalreg, string in_interface,
									 vector<opt_pair> *in_opts) :
	KisPacketSource(in_globalreg, in_interface, in_opts) {

	serial_fd = -1;
	sync = false;
	scan_mode = -1;

	ParseOptions(in_opts);
}

PacketSource_Dect::~PacketSource_Dect() {
	if (serial_fd >= 0) {
		close(serial_fd);
		serial_fd = -1;
	}
}

int PacketSource_Dect::AutotypeProbe(string in_device) {
	if (StrLower(in_device) == "dect") {
		type = "dect";
		return 1;
	}

	if (StrLower(in_device) == "dectus") {
		type = "dectus";
		return 1;
	}

	return 0;
}

int PacketSource_Dect::ParseOptions(vector<opt_pair> *in_opts) {
	if ((serialdevice = FetchOpt("device", in_opts)) == "") {
		_MSG("DECT packet source defaulting to default serial /dev/coa",
			 MSGFLAG_INFO);
		serialdevice = "/dev/coa";
	} else {
		_MSG("DECT packet source using serial device " + serialdevice,
			 MSGFLAG_INFO);
	}

	return 1;
}

int PacketSource_Dect::FetchDescriptor() {
	return serial_fd;
}

int PacketSource_Dect::SetChannel(unsigned int in_ch) {
	if (ioctl(serial_fd, COA_IOCTL_CHAN, &in_ch)) {
		_MSG("DECT packet source '" + name + "': Failed to set channel: " +
			 string(strerror(errno)), MSGFLAG_ERROR);
		return 0;
	}

	last_channel = in_ch;

	// printf("debug - dect set channel %u\n", in_ch);

	return 1;
}

int PacketSource_Dect::StartScanPp(mac_addr in_rfpi) {
	if (serial_fd == -1)
		return -1;

	/* always accept mode set since we might change RFPI */
	/* remove the sync early since we've disrupted the state of the world */

	if (sync) {
		sync = false;
	}

	/* Set sync sniff mode */
	uint16_t val;
	val = COA_MODE_SNIFF | COA_SUBMODE_SNIFF_SYNC;
	if (ioctl(serial_fd, COA_IOCTL_MODE, &val)) {
		_MSG("DECT packet source '" + name + "': Failed to set sniffer mode PP: " +
			 string(strerror(errno)), MSGFLAG_ERROR);
		return -1;
	}

	/* set RFPI to sync with */
	uint8_t mrfpi[5];
	for (unsigned int x = 0; x < 5; x++)
		mrfpi[x] = in_rfpi[x];

	if (ioctl(serial_fd, COA_IOCTL_SETRFPI, mrfpi)) {
		_MSG("DECT packet source '" + name + "': Failed to set PP RFPI: " +
			 string(strerror(errno)), MSGFLAG_ERROR);
		return -1;
	}

	scan_mode = MODE_ASYNC_PP_SCAN;

	// Caller should turn off PST channel hopping and set channel
	// globalreg->sourcetracker->SetSourceHopping(FetchUUID(), 0, 0);

	return 1;
}

int PacketSource_Dect::StartScanCalls(mac_addr in_rfpi, int in_channel) {
	if (serial_fd == -1)
		return -1;

	if (sync) 
		sync = false;

	/* Set sync sniff mode */
	uint16_t val;

	val = COA_MODE_SNIFF | COA_SUBMODE_SNIFF_SYNC;

	if (ioctl(serial_fd, COA_IOCTL_MODE, &val)) {
		_MSG("DECT packet source '" + name + "': Failed to set sniffer mode SYNC: " +
			 string(strerror(errno)), MSGFLAG_ERROR);
		return -1;
	}

	/* set RFPI we're syncing to */
	uint8_t mrfpi[5];
	for (unsigned int x = 0; x < 5; x++)
		mrfpi[x] = in_rfpi[x];

	if (ioctl(serial_fd, COA_IOCTL_SETRFPI, mrfpi)) {
		_MSG("DECT packet source '" + name + "': Failed to set sniffer sync RFPI: " +
			 string(strerror(errno)), MSGFLAG_ERROR);
		return -1;
	}

	scan_mode = MODE_SYNC_CALL_SCAN;

	// Caller should turn off PST channel hopping
	// globalreg->sourcetracker->SetSourceHopping(FetchUUID(), 0, in_channel);

	return 1;
}

int PacketSource_Dect::StartScanFp() {
	if (serial_fd == -1)
		return -1;

	if (scan_mode == MODE_ASYNC_FP_SCAN)
		return 0;

	/* start sniffer mode */
	int val = COA_MODE_SNIFF | COA_SUBMODE_SNIFF_SCANFP;

	if (ioctl(serial_fd, COA_IOCTL_MODE, &val)) {
		_MSG("DECT packet source '" + name + "': Failed to set sniffer scan FP: " +
			 string(strerror(errno)), MSGFLAG_ERROR);
		return -1;
	}

	scan_mode = MODE_ASYNC_FP_SCAN;

	if (sync) 
		sync = false;

	// Caller should turn on hopping
	// globalreg->sourcetracker->SetSourceHopping(FetchUUID(), 1, 0);

	return 1;
}

int PacketSource_Dect::OpenSource() {
	if ((serial_fd = open(serialdevice.c_str(), O_RDONLY)) == -1) {
		_MSG("DECT packet source '" + name + "': Failed to open DECT serial device: " +
			 string(strerror(errno)), MSGFLAG_ERROR);
		return 0;
	}

	return StartScanFp();
}

int PacketSource_Dect::CloseSource() {
	if (serial_fd >= 0) {
		close(serial_fd);
		serial_fd = -1;
	}

	return 1;
}

int PacketSource_Dect::Poll() {
	unsigned char buf[7];
	int rbytes = -1;
	dect_packinfo *pi = new dect_packinfo;
	kis_packet *newpack;
	kis_datachunk *dectchunk = NULL; 

	pi->sync = sync;
	pi->scanmode = scan_mode;
	pi->channel = FetchChannel();

	if (scan_mode == MODE_ASYNC_FP_SCAN || scan_mode == MODE_ASYNC_PP_SCAN) {
		if ((rbytes = read(serial_fd, &(pi->sdata), sizeof(dect_data_scan_t))) !=
			 sizeof(dect_data_scan_t)) {
			_MSG("DECT packet source '" + name + "': Failed to read scan data: " +
				 string(strerror(errno)), MSGFLAG_ERROR);
			delete(pi);
			return 0;
		}

		// Make a scan chunk
		dectchunk = new kis_datachunk;
		dectchunk->length = sizeof(dect_data_scan_t);
		dectchunk->data = new uint8_t[dectchunk->length];
		memcpy(dectchunk->data, &(pi->sdata), dectchunk->length);
		dectchunk->source_id = source_id;
		dectchunk->dlt = KDLT_DECTSCAN;

	} else if (scan_mode == MODE_SYNC_CALL_SCAN) {
		if ((rbytes = read(serial_fd, &(pi->pdata), sizeof(pp_packet_t))) !=
			 sizeof(pp_packet_t)) {
			_MSG("DECT packet source '" + name + "': Failed to read scan data: " +
				 string(strerror(errno)), MSGFLAG_ERROR);
			delete(pi);
			return 0;
		} else {
			if (!sync) {
				_MSG("DECT packet source '" + name + "': Synced to call", MSGFLAG_INFO);
				sync = true;
			}

			// Make a data chunk
			dectchunk = new kis_datachunk;
			dectchunk->length = 53;
			dectchunk->data = new uint8_t[dectchunk->length];
			memcpy(dectchunk->data, pi->pdata.data, dectchunk->length);
			dectchunk->source_id = source_id;
			dectchunk->dlt = KDLT_DECTCALL;
		}
	} else {
		// Unknown mode during poll
		delete(pi);
		return 0;
	}

	newpack = globalreg->packetchain->GeneratePacket();

	kis_ref_capsource *csrc_ref = new kis_ref_capsource;
	csrc_ref->ref_source = this;
	newpack->insert(_PCM(PACK_COMP_KISCAPSRC), csrc_ref);

	newpack->insert(pack_comp_dect, pi);
	if (dectchunk) 
		newpack->insert(_PCM(PACK_COMP_LINKFRAME), dectchunk);
	globalreg->packetchain->ProcessPacket(newpack);

	// printf("debug - dect newpack\n");

	return 1;
}

