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
#include <globalregistry.h>

#include "dumpfile_decttxt.h"

Dumpfile_Decttxt::Dumpfile_Decttxt(GlobalRegistry *in_globalreg, 
								   Tracker_Dect *in_tracker) : Dumpfile(in_globalreg) {

	globalreg = in_globalreg;
	tracker = in_tracker;

	txtfile = NULL;

	if ((fname = ProcessConfigOpt("decttxt")) == "" ||
		globalreg->fatal_condition) {
		return;
	}

	if ((txtfile = fopen(fname.c_str(), "w")) == NULL) {
		_MSG("Failed to open decttxt log file '" + fname + "': " + strerror(errno),
			 MSGFLAG_FATAL);
		globalreg->fatal_condition = 1;
		return;
	}

	globalreg->RegisterDumpFile(this);

	_MSG("Opened decttxt log file '" + fname + "'", MSGFLAG_INFO);
}

Dumpfile_Decttxt::~Dumpfile_Decttxt() {
	Flush();
}

int Dumpfile_Decttxt::Flush() {
	if (txtfile != NULL)
		fclose(txtfile);

	string tempname = fname + ".temp";
	if ((txtfile = fopen(tempname.c_str(), "w")) == NULL) {
		_MSG("Failed to open temporary nettxt file for writing: " +
			 string(strerror(errno)), MSGFLAG_ERROR);
		return -1;
	}

	fprintf(txtfile, "Kismet (http://www.kismetwireless.net)\n"
			"Dedected (http://www.dedected.org)\n"
			"%.24s - Kismet %s.%s.%s\n"
			"-----------------\n\n",
			ctime(&(globalreg->start_time)),
			globalreg->version_major.c_str(),
			globalreg->version_minor.c_str(),
			globalreg->version_tiny.c_str());

	map<mac_addr, dect_tracked_fp *> m = tracker->tracked_fp;

	int count = 1;

	for (map<mac_addr, dect_tracked_fp *>::iterator i = m.begin(); 
		 i != m.end(); ++i) {

		fprintf(txtfile, "Basestation %d: RFPI %s\n", count,
				i->second->rfpi.Mac2String().substr(0, 14).c_str());
		fprintf(txtfile, " First      : %.24s\n", ctime(&(i->second->first_time)));
		fprintf(txtfile, " Last       : %.24s\n", ctime(&(i->second->last_time)));
		fprintf(txtfile, " RFPI       : %s\n", 
				i->second->rfpi.Mac2String().substr(0, 14).c_str());
		fprintf(txtfile, " Channel    : %u\n", i->second->channel);
		fprintf(txtfile, " Count      : %u\n", i->second->num_seen);
		if (i->second->min_rssi > 0)
			fprintf(txtfile, " Min RSSI   : %d\n", i->second->min_rssi);
		if (i->second->max_rssi > 0)
			fprintf(txtfile, " Max RSSI   : %d\n", i->second->max_rssi);

		if (i->second->gpsdata.gps_valid) {
			fprintf(txtfile, " Min Pos    : Lat %f Lon %f Alt %f Spd %f\n", 
					i->second->gpsdata.min_lat, i->second->gpsdata.min_lon,
					i->second->gpsdata.min_alt, i->second->gpsdata.min_spd);
			fprintf(txtfile, " Max Pos    : Lat %f Lon %f Alt %f Spd %f\n", 
					i->second->gpsdata.max_lat, i->second->gpsdata.max_lon,
					i->second->gpsdata.max_alt, i->second->gpsdata.max_spd);
			fprintf(txtfile, " Peak Pos   : Lat %f Lon %f Alt %f\n", 
					i->second->peak_lat, i->second->peak_lon,
					i->second->peak_alt);
			fprintf(txtfile, " Avg Pos    : AvgLat %Lf AvgLon %Lf AvgAlt %Lf\n",
					i->second->gpsdata.aggregate_lat / 
						i->second->gpsdata.aggregate_points,
					i->second->gpsdata.aggregate_lon / 
						i->second->gpsdata.aggregate_points,
					i->second->gpsdata.aggregate_alt / 
						i->second->gpsdata.aggregate_points);

		}

		fprintf(txtfile, "\n");

		count++;
	}

	dumped_frames = count;

	fflush(txtfile);
	fclose(txtfile);

	txtfile = NULL;

	if (rename(tempname.c_str(), fname.c_str()) < 0) {
		_MSG("Failed to rename decttxt temp file " + tempname + " to " + fname + ":" +
			 string(strerror(errno)), MSGFLAG_ERROR);
		return -1;
	}

	return 1;
}
