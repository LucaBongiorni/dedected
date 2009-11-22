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

#ifndef __DUMPFILE_DECTTXT_H__
#define __DUMPFILE_DECTTXT_H__

#include "config.h"

#include <packet.h>
#include <gpscore.h>
#include <configfile.h>
#include <dumpfile.h>

#include "packet_dect.h"
#include "tracker_dect.h"

class Dumpfile_Decttxt : public Dumpfile {
public:
	Dumpfile_Decttxt() { fprintf(stderr, "FATAL OOPS: Dumpfile_Decttxt()\n"); exit(1); }
	Dumpfile_Decttxt(GlobalRegistry *in_globalreg, Tracker_Dect *in_tracker);
	virtual ~Dumpfile_Decttxt();

	virtual int Flush();

protected:
	FILE *txtfile;
	Tracker_Dect *tracker;
};

#endif

