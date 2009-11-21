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

#ifndef __PACKETSOURCE_DECT_H__
#define __PACKETSOURCE_DECT_H__

#include "config.h"

#include <packetsource.h>
#include <packetsourcetracker.h>

#define USE_PACKETSOURCE_DECT

#define MODE_ASYNC_FP_SCAN  0
#define MODE_ASYNC_PP_SCAN  1
#define MODE_SYNC_CALL_SCAN 2

#define COA_IOCTL_MODE                  0xD000
#define COA_IOCTL_CHAN                  0xD004
#define COA_IOCTL_SETRFPI               0xD008
#define COA_MODE_SNIFF                  0x0300
#define COA_SUBMODE_SNIFF_ALL           0x0000
#define COA_SUBMODE_SNIFF_SCANFP        0x0001
#define COA_SUBMODE_SNIFF_SCANPP        0x0002
#define COA_SUBMODE_SNIFF_SYNC          0x0003


/* DECT Client Command Defs */
#define DECT_CMD_START              0
#define DECT_CMD_END                9
#define DECT_SUBCMD_START           0
#define DECT_SUBCMD_END             9

/* Commands */
#define DECT_CMD_CHANHOP            0
#define DECT_CMD_SCAN               1

/* Subcommands */
#define DECT_SUBCMD_CHANHOP_ENABLE  1
#define DECT_SUBCMD_CHANHOP_DISABLE 0
#define DECT_SUBCMD_SCAN_FP         0
#define DECT_SUBCMD_SCAN_PP         1
#define DECT_SUBCMD_SCAN_CALLS      2

class PacketSource_Dect : public KisPacketSource {
public:
	PacketSource_Dect() {
		fprintf(stderr, "FATAL OOPS: Packetsource_Raven()\n");
		exit(1);
	}

	PacketSource_Dect(GlobalRegistry *in_globalreg); 

	virtual KisPacketSource *CreateSource(GlobalRegistry *in_globalreg,
										  string in_interface,
										  vector<opt_pair> *in_opts) {
		return new PacketSource_Dect(in_globalreg, in_interface, in_opts);
	}

	virtual int AutotypeProbe(string in_device);

	virtual int RegisterSources(Packetsourcetracker *tracker) {
		tracker->RegisterPacketProto("dect", this, "DECT", 0);
		tracker->RegisterPacketProto("dectus", this, "DECTUS", 0);
		return 1;
	}

	PacketSource_Dect(GlobalRegistry *in_globalreg, string in_interface,
					   vector<opt_pair> *in_opts);

	virtual ~PacketSource_Dect();

	virtual int ParseOptions(vector<opt_pair> *in_opts);

	virtual int OpenSource();
	virtual int CloseSource();

	virtual int FetchChannelCapable() { return 1; }
	virtual int EnableMonitor() { return 1; }
	virtual int DisableMonitor() { return 1; }

	virtual int SetChannel(unsigned int in_ch);

	virtual int FetchDescriptor();
	virtual int Poll();

	int StartScanFp();
	int StartScanPp(mac_addr in_rfpi);
	int StartScanCalls(mac_addr in_rfpi, int in_channel);

protected:
	virtual void FetchRadioData(kis_packet *in_packet) { };

	int dect_packet_id;

	int scan_mode;

	string serialdevice;
	int serial_fd;
	bool sync;

};

#endif
