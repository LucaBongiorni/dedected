/*
 * dect_cli async and sync interface to DECT, can dump pcap files
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * authors:
 * (C) 2008  Matthias Wenzel <dect at mazzoo dot de>
 * (C) 2008  Andreas Schuler <krater at badterrorist dot com>
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <time.h>
#include <limits.h>
#include <signal.h>
#include <ctype.h>
#include <pcap.h>


#include "com_on_air_user.h"
#include "dect_cli.h"

struct cli_info cli;


#define  RXBUF 8192
char buf[RXBUF];


/* pcap errors */
char errbuf[PCAP_ERRBUF_SIZE];

int rfpi_is_ignored(const uint8_t * RFPI);

void print_help(void)
{
	LOG("\n");
	LOG("   help          - this help\n");
	LOG("   fpscan        - async scan for basestations, dump RFPIs\n");
	LOG("   callscan      - async scan for active calls, dump RFPIs\n");
	LOG("   autorec       - sync on any calls in callscan, autodump in pcap\n");
	LOG("   ppscan <rfpi> - sync scan for active calls\n");
	LOG("   chan <ch>     - set current channel [0-9], currently %d\n", cli.channel);
//	LOG("   slot <sl>     - set current slot [0-23], currently %d\n", cli.slot);
//	LOG("   jam           - jam current channel\n");
	LOG("   band          - toggle between EMEA/DECT and US/DECT6.0 bands\n");
	LOG("   ignore <rfpi> - toggle ignoring of an RFPI in autorec\n");
	LOG("   dump          - dump stations and calls we have seen\n");
	LOG("   hop           - toggle channel hopping, currently %s\n", cli.hop ? "ON":"OFF");
	LOG("   verb          - toggle verbosity, currently %s\n", cli.verbose ? "ON":"OFF");
	LOG("   stop          - stop it - whatever we were doing\n");
	LOG("   quit          - well :)\n");
	LOG("\n");
}

void set_channel(uint32_t channel)
{
	if (cli.verbose)
		LOG("### switching to channel %d\n", ch2etsi[channel]);
	if (ioctl(cli.fd, COA_IOCTL_CHAN, &ch2etsi[channel])){
		LOG("!!! couldn't ioctl()\n");
		exit(1);
	}
	cli.last_hop = time(NULL);
}

void set_slot(uint32_t slot)
{
	LOG("!!! not yet implemented :(\n");
}

void do_ppscan(uint8_t * RFPI)
{
	LOG("### trying to sync on %.2x %.2x %.2x %.2x %.2x\n",
		RFPI[0],
		RFPI[1],
		RFPI[2],
		RFPI[3],
		RFPI[4]
	   );

	/* set sync sniff mode */
	uint16_t val;
	val = COA_MODE_SNIFF | COA_SUBMODE_SNIFF_SYNC;
	if (ioctl(cli.fd, COA_IOCTL_MODE, &val)){
		LOG("!!! couldn't ioctl()\n");
		exit(1);
	}

	/* set rfpi to sync with */
	if(ioctl(cli.fd, COA_IOCTL_SETRFPI, RFPI)){
		LOG("!!! couldn't ioctl()\n");
		exit(1);
	}

	set_channel(cli.channel);

	memcpy(cli.RFPI, RFPI, 5);
	cli.mode = MODE_PPSCAN;

	cli.autorec_last_bfield = time(NULL);
}

void add_station(struct dect_station * station)
{
	int i;
	LOG("### found new %s", station->type == TYPE_FP ? "station":"call on");
	for (i=0; i<5; i++)
		LOG(" %.2x", station->RFPI[i]);
	LOG(" on channel %d RSSI %d\n", station->channel, station->RSSI);

	struct dect_station * p = cli.station_list;
	if (p)
	{ /* append to existing list */
		while (p->next)
			p = p->next;
		p->next = malloc(sizeof(*p));
		p = p->next;
	}else /* create 1st element in list */
	{
		cli.station_list = malloc(sizeof(*cli.station_list));
		p = cli.station_list;
	}
	if (!p)
	{
		LOG("!!! out of memory\n");
		exit(1);
	}
	memset(p, 0, sizeof(*p));

	memcpy(p->RFPI, station->RFPI, 5);
	p->channel = station->channel;
	p->RSSI = station->RSSI;
	p->type = station->type;
	p->first_seen = time(NULL);
	p->last_seen = p->first_seen;
	p->count_seen = 1;
}

void try_add_station(struct dect_station * station)
{
	struct dect_station * p = cli.station_list;
	int found = 0;
	while (p)
	{
		if (!memcmp(p->RFPI, station->RFPI, 5))
		{
			if (p->type == station->type)
			{
				if ( (p->channel != station->channel) &&
						(cli.verbose) )
				{
					int i;
					LOG("### station");
					for (i=0; i<5; i++)
						LOG(" %.2x", station->RFPI[i]);
					LOG(" switched from channel %d to channel %d\n",
							p->channel,
							station->channel);
				}
				found = 1;
				p->channel = station->channel;
				p->count_seen++;
				p->last_seen = time(NULL);
				p->RSSI += station->RSSI; /* we avg on dump */
			}
		}
		p = p->next;
	}
	if (!found)
		add_station(station);
	if (cli.autorec && (cli.mode != MODE_PPSCAN))
	{
		if (rfpi_is_ignored(station->RFPI))
		{
			if (cli.verbose)
			{
				LOG("### skipping ignored RFPI %.2x %.2x %.2x %.2x %.2x\n",
						station->RFPI[0], station->RFPI[1], station->RFPI[2],
						station->RFPI[3], station->RFPI[4]);
			}
		}
		else
		{
			do_ppscan(station->RFPI);
		}
	}
}


void do_fpscan(void)
{
	LOG("### starting fpscan\n");
	uint16_t val;
	val = COA_MODE_SNIFF | COA_SUBMODE_SNIFF_SCANFP;
	if (ioctl(cli.fd, COA_IOCTL_MODE, &val)){
		LOG("!!! couldn't ioctl()\n");
		exit(1);
	}
	/* set start channel */
	set_channel(cli.channel);
	cli.mode = MODE_FPSCAN;
	cli.autorec = 0;
}

void do_callscan(void)
{
	LOG("### starting callscan\n");
	uint16_t val;
	val = COA_MODE_SNIFF | COA_SUBMODE_SNIFF_SCANPP;
	if (ioctl(cli.fd, COA_IOCTL_MODE, &val)){
		LOG("!!! couldn't ioctl()\n");
		exit(1);
	}
	/* set start channel */
	set_channel(cli.channel);
	cli.mode = MODE_CALLSCAN;
}

int hexvalue(int hexdigit)
{
	if (hexdigit >= '0' && hexdigit <= '9')
		return hexdigit - '0';
	if (hexdigit >= 'a' && hexdigit <= 'f')
		return hexdigit - ('a' - 10);
	if (hexdigit >= 'A' && hexdigit <= 'F')
		return hexdigit - ('A' - 10);
	return -1;
}

int parse_rfpi(const char * str_rfpi, uint8_t * rfpi)
{
	int i = 0;

	// Skip initial whitespace:
	while (isspace(*str_rfpi))
		str_rfpi++;

	for (;;)
	{
		int highnibble, lownibble;
		highnibble = hexvalue(str_rfpi[0]);
		// Need to check for validity of the first character before
		// continuing with the next, in case the first one was \0:
		if (highnibble == -1)
			return -1;
		lownibble = hexvalue(str_rfpi[1]);
		if (lownibble == -1)
			return -1;
		rfpi[i] = (highnibble << 4) | lownibble;
		
		if (i == 4)
			break;
		i++;
		str_rfpi += 2;

		// Accept space or colon as byte separator. None at all is ok too.
		if (*str_rfpi == ' ' || *str_rfpi == ':')
			str_rfpi++;
	}

	return 0;
}

void do_ppscan_str(char * str_rfpi)
{
	uint8_t RFPI[5];

	if (parse_rfpi(str_rfpi, RFPI) == -1)
	{
		LOG("!!! please enter a valid RFPI (e.g. 00 01 02 03 04)\n");
		return;
	}
	do_ppscan(RFPI);
}

// Returns true if 'RFPI' occurs in 'list'.
int rfpi_list_present(struct rfpi_list * list, const uint8_t * RFPI)
{
	for (; list != NULL; list = list->next)
	{
		if (!memcmp(RFPI, list->RFPI, 5))
			return 1;
	}
	return 0;
}

// Adds 'RFPI' at the front of '*list_ptr'.
void rfpi_list_add(struct rfpi_list ** list_ptr, const uint8_t * RFPI)
{
	struct rfpi_list * new_link = malloc(sizeof (struct rfpi_list));
	memcpy(new_link->RFPI, RFPI, 5);
	new_link->next = *list_ptr;
	*list_ptr = new_link;
}

// Removes the first occurence of 'RFPI' from '*list_ptr'.
void rfpi_list_remove(struct rfpi_list ** list_ptr, const uint8_t * RFPI)
{
	for (; *list_ptr != NULL; list_ptr = &(*list_ptr)->next)
	{
		struct rfpi_list * link = *list_ptr;
		if (!memcmp(RFPI, link->RFPI, 5))
		{
			*list_ptr = link->next;
			free(link);
			return;
		}
	}
}

int rfpi_is_ignored(const uint8_t * RFPI)
{
	return rfpi_list_present(cli.ignored_rfpis, RFPI);
}

void rfpi_ignore_rfpi(const uint8_t * RFPI)
{
	rfpi_list_add(&cli.ignored_rfpis, RFPI);
}

void rfpi_unignore_rfpi(const uint8_t * RFPI)
{
	rfpi_list_remove(&cli.ignored_rfpis, RFPI);
}

void do_ignore_str(const char * str_rfpi)
{
	uint8_t RFPI[5];

	if (parse_rfpi(str_rfpi, RFPI) == -1)
	{
		LOG("!!! please enter a valid RFPI (e.g. 00 01 02 03 04)\n");
		return;
	}

	if (rfpi_is_ignored(RFPI))
	{
		LOG("### no longer ignoring RFPI %.2x %.2x %.2x %.2x %.2x\n",
				RFPI[0], RFPI[1], RFPI[2], RFPI[3], RFPI[4]);
		rfpi_unignore_rfpi(RFPI);
	}
	else
	{
		LOG("### ignoring RFPI %.2x %.2x %.2x %.2x %.2x\n",
				RFPI[0], RFPI[1], RFPI[2], RFPI[3], RFPI[4]);
		rfpi_ignore_rfpi(RFPI);
	}
}


void do_chan(char * str_chan)
{
	uint32_t channel;
	char * end;
	errno = 0;
	channel = strtoul(str_chan, &end, 0);
	if ((errno == ERANGE && (channel == LONG_MAX || channel == LONG_MIN))
			|| (errno != 0 && channel == 0))
	{
		LOG("!!! please enter a valid channel number [0-14]\n");
		return;
	}
	if (end == str_chan)
	{
		LOG("!!! please enter a valid channel number [0-14]\n");
		return;
	}
	if (! ((channel >=  0) && (channel <= 14)) )
	{
		LOG("!!! please enter a valid channel number [0-14]\n");
		return;
	}

	if(channel<10)
		channel = 9 - channel;

	cli.channel = channel;
	set_channel(cli.channel);
}

void do_slot(char * str_slot)
{
	uint32_t slot;
	char * end;
	errno = 0;
	slot = strtoul(str_slot, &end, 0);
	if ((errno == ERANGE && (slot == LONG_MAX || slot == LONG_MIN))
			|| (errno != 0 && slot == 0))
	{
		LOG("!!! please enter a valid slot number [0-23]\n");
		return;
	}
	if (end == str_slot)
	{
		LOG("!!! please enter a valid slot number [0-23]\n");
		return;
	}
	if (slot > 23)
	{
		LOG("!!! please enter a valid slot number [0-23]\n");
		return;
	}
	cli.slot = slot;
	set_slot(cli.slot);
}

void do_jam(void)
{
	LOG("!!! not yet implemented :(\n");
}

void do_band(void)
{
	switch (cli.band)
	{
		case DECT_BAND_EMEA:
			cli.band      = DECT_BAND_US;
			cli.hop_start = 10;
			cli.hop_end   = 14;
			cli.channel   = cli.hop_start;
			LOG("### using US/DECT6.0 band\n");
			break;
		case DECT_BAND_US:
			cli.band      = DECT_BAND_EMEA | DECT_BAND_US;
			cli.hop_start = 0;
			cli.hop_end   = 14;
			cli.channel   = cli.hop_start;
			LOG("### using both EMEA/DECT and US/DECT6.0 band\n");
			break;
		case DECT_BAND_EMEA | DECT_BAND_US:
			cli.band      = DECT_BAND_EMEA;
			cli.hop_start = 0;
			cli.hop_end   = 9;
			cli.channel   = cli.hop_start;
			LOG("### using EMEA/DECT band\n");
			break;
	}

}

void do_dump(void)
{
	int i;
	struct dect_station * p = cli.station_list;
	struct rfpi_list * r = cli.ignored_rfpis;
	if (!p)
	{
		LOG("### nothing found so far\n");
		goto dump_ignore;
	}

	LOG("### stations\n");
	do
	{
		if (p->type == TYPE_FP)
		{
			LOG("   ");
			for (i=0; i<5; i++)
				LOG(" %.2x", p->RFPI[i]);
			LOG("  ch %1.1d ", p->channel);
			LOG(" RSSI %5.2f ", (double)p->RSSI / p->count_seen);
			LOG(" count %4.u ", p->count_seen);
			LOG(" first %u ", p->first_seen);
			LOG(" last %u ", p->last_seen);
			LOG("\n");
		}
	} while ((p = p->next));

	p = cli.station_list;
	LOG("### calls\n");
	do
	{
		if (p->type == TYPE_PP)
		{
			LOG("   ");
			for (i=0; i<5; i++)
				LOG(" %.2x", p->RFPI[i]);
			LOG("  ch %1.1d ", p->channel);
			LOG(" RSSI %5.2f ", (double)p->RSSI / p->count_seen);
			LOG(" count %4.u ", p->count_seen);
			LOG(" first %u ", p->first_seen);
			LOG(" last %u ", p->last_seen);
			LOG("\n");
		}
	} while ((p = p->next));

dump_ignore:
	if (!r)
		return;

	LOG("### RFPIs ignored\n");
	do{
		LOG("   %.2x %.2x %.2x %.2x %.2x is ignored\n",
			r->RFPI[0],
			r->RFPI[1],
			r->RFPI[2],
			r->RFPI[3],
			r->RFPI[4]
			);
	} while ((r = r->next));
}

void do_hop(void)
{
	cli.hop = cli.hop ? 0:1;
	LOG("### channel hopping turned %s\n", cli.hop ? "ON":"OFF");
}

void do_verb(void)
{
	cli.verbose = cli.verbose ? 0:1;
	LOG("### verbosity turned %s\n", cli.verbose ? "ON":"OFF");
}

void do_autorec(void)
{
	cli.autorec = cli.autorec ? 0:1;
	LOG("### starting autorec\n");
}

void do_stop_keep_autorec(void)
{
	LOG("### stopping DIP\n");
	uint16_t val;
	val = COA_MODE_IDLE;
	if (ioctl(cli.fd, COA_IOCTL_MODE, &val)){
		LOG("couldn't ioctl()\n");
		exit(1);
	}
	cli.mode = MODE_STOP;
}

void do_stop(void)
{
	if (!(cli.mode & MODE_STOP))
	{
		do_stop_keep_autorec();
	}
	cli.autorec = 0;
}

void do_quit(void)
{
	do_stop();
	do_dump();
	exit(0);
}

void process_cli_data()
{
	int ret;
	ret = read(cli.in, buf, RXBUF);
	buf[ret]=0;
	if(buf[ret-1] == '\n')
		buf[ret-1] = 0;
	int done = 0;
	if ( !strncasecmp((char *)buf, "help", 4) )
		{ print_help(); done = 1; }
	if ( !strncasecmp((char *)buf, "fpscan", 6) )
		{ do_fpscan(); done = 1; }
	if ( !strncasecmp((char *)buf, "callscan", 8) )
		{ do_callscan(); done = 1; }
	if ( !strncasecmp((char *)buf, "autorec", 7) )
		{ do_autorec(); done = 1; }
	if ( !strncasecmp((char *)buf, "ppscan", 6) )
		{ do_ppscan_str(&buf[6]); done = 1; }
	if ( !strncasecmp((char *)buf, "chan", 4) )
		{ do_chan(&buf[4]); done = 1; }
	if ( !strncasecmp((char *)buf, "slot", 4) )
		{ do_slot(&buf[4]); done = 1; }
	if ( !strncasecmp((char *)buf, "jam", 3) )
		{ do_jam(); done = 1; }
	if ( !strncasecmp((char *)buf, "band", 4) )
		{ do_band(); done = 1; }
	if ( !strncasecmp((char *)buf, "ignore", 6) )
		{ do_ignore_str(&buf[6]); done = 1; }
	if ( !strncasecmp((char *)buf, "dump", 4) )
		{ do_dump(); done = 1; }
	if ( !strncasecmp((char *)buf, "hop", 3) )
		{ do_hop(); done = 1; }
	if ( !strncasecmp((char *)buf, "verb", 4) )
		{ do_verb(); done = 1; }
	if ( !strncasecmp((char *)buf, "stop", 4) )
		{ do_stop(); done = 1; }
	if ( !strncasecmp((char *)buf, "quit", 4) )
		do_quit();

	if(!done)
		LOG("!!! no such command %s\n", buf);

}

void init_pcap(struct sniffed_packet * packet)
{
	char fname[512];
	char ftime[256];
	time_t rawtime;
	struct tm *timeinfo;

	time (&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(ftime, sizeof(ftime), "%Y-%m-%d_%H_%M_%S", timeinfo);

	sprintf(fname, "dump_%s_RFPI_%.2x_%.2x_%.2x_%.2x_%.2x.pcap",
			ftime,
			cli.RFPI[0],
			cli.RFPI[1],
			cli.RFPI[2],
			cli.RFPI[3],
			cli.RFPI[4]);
	LOG("### dumping to %s\n", fname);
	cli.pcap = pcap_open_dead(DLT_EN10MB, 73);
	if (!cli.pcap)
	{
		LOG("!!! couldn't pcap_open_dead(\"%s\")\n", fname);
	}
	cli.pcap_d = pcap_dump_open(cli.pcap, fname);
	if (!cli.pcap_d)
	{
		LOG("!!! couldn't pcap_dump_open(\"%s\")\n", fname);
	}
}

int has_b_field()
{
	if ((cli.packet.data[5] & 0x0e) != 0x0e)
		return 1;
	return 0;
}

void process_dect_data()
{
	int ret;
	switch (cli.mode)
	{
		case MODE_FPSCAN:
			while (7 == (ret = read(cli.fd, buf, 7))){
				memcpy(cli.station.RFPI, &buf[2], 5);
				cli.station.channel = buf[0];
				cli.station.RSSI = buf[1];
				cli.station.type = TYPE_FP;
				try_add_station(&cli.station);
			}
			break;
		case MODE_CALLSCAN:
			while (7 == (ret = read(cli.fd, buf, 7))){
				memcpy(cli.station.RFPI, &buf[2], 5);
				cli.station.channel = buf[0];
				cli.station.RSSI = buf[1];
				cli.station.type = TYPE_PP;
				try_add_station(&cli.station);
			}
			break;
		case MODE_PPSCAN:
			while ( sizeof(cli.packet) ==
			        read(cli.fd, &cli.packet, sizeof(cli.packet)))
			{
				memcpy(cli.station.RFPI, cli.RFPI, 5);
				cli.station.channel = cli.packet.channel;
				cli.station.RSSI = cli.packet.rssi;
				cli.station.type = TYPE_PP;
				/* to ypdate statistics only we try_add_station() */
				try_add_station(&cli.station);

				/* stop hopping once we're synchronized */
				cli.hop = 0;

				if (!cli.pcap)
				{
					LOG("### got sync\n");
					init_pcap(&cli.packet);
					/* this is not actually a B-Field,
					 * but we expect some to come soon
					 * and the val needs to be non-0 */
					cli.autorec_last_bfield = time(NULL);
				}
				if (has_b_field())
					cli.autorec_last_bfield = time(NULL);

				struct pcap_pkthdr pcap_hdr;
				pcap_hdr.caplen = 73;
				pcap_hdr.len = 73;
				ret = gettimeofday(&pcap_hdr.ts, NULL);
				if (ret)
				{
					LOG("!!! couldn't gettimeofday(): %s\n",
							strerror(errno));
					exit(1);
				}
				uint8_t pcap_packet[100];
				memset(pcap_packet, 0, 100);
				pcap_packet[12] = 0x23;
				pcap_packet[13] = 0x23;
				pcap_packet[14] = 0x00;        /* decttype (receive) */
				pcap_packet[15] = cli.packet.channel;
				pcap_packet[16] = 0x00;
				pcap_packet[17] = cli.packet.slot;
				pcap_packet[18] = cli.packet.framenumber;
				pcap_packet[19] = cli.packet.rssi;
				memcpy(&pcap_packet[20], cli.packet.data, 53);

				pcap_dump(cli.pcap_d, &pcap_hdr, pcap_packet);
			}
			break;
	}
}

void init_dect()
{
	cli.fd = open(DEV, O_RDWR | O_NONBLOCK);
	if (cli.fd < 0)
	{
		LOG("!!! couldn't open(\"%s\"): %s\n",
				DEV,
				strerror(errno));
		exit(1);
	}
	cli.pcap = NULL;
}

void signal_handler(int s)
{
	LOG("### got signal %d, will dump & quit\n", s);
	do_quit();
}

void init_cli()
{
	cli.channel      = 0;
	cli.slot         = 0;
	cli.hop          = 1;
	cli.hop_ch_time  = 1; /* in sec */

	cli.mode         = MODE_STOP;

	cli.in           = fileno(stdin);

	cli.verbose      = 0;

	cli.station_list = NULL;
	cli.ignored_rfpis= NULL;

	cli.autorec             = 0;
	cli.autorec_timeout     = 10;
	cli.autorec_last_bfield = 0;

	cli.band                = DECT_BAND_EMEA;
	cli.hop_start           = 0;
	cli.hop_end             = 9;

	signal(SIGHUP, signal_handler);
	signal(SIGINT, signal_handler);
	signal(SIGQUIT, signal_handler);
	signal(SIGABRT, signal_handler);
	signal(SIGKILL, signal_handler);
	signal(SIGALRM, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGUSR1, signal_handler);
	signal(SIGUSR2, signal_handler);
}

void init(void)
{
	init_dect();
	init_cli();
}

int max_int(int a, int b)
{
	if (a>b)
		return a;
	else
		return b;
}

void mainloop(void)
{
	fd_set rfd;
	fd_set wfd;
	fd_set efd;

	int nfds = max_int(cli.in, cli.fd);
	nfds++;

	struct timeval tv;

	int ret;

	while (0xDEC + 'T')
	{
		tv.tv_sec  = 1;
		tv.tv_usec = 0;

		FD_ZERO(&rfd);
		FD_ZERO(&wfd);
		FD_ZERO(&efd);

		FD_SET(cli.in, &rfd);
		FD_SET(cli.fd, &rfd);

		FD_SET(cli.in, &efd);
		FD_SET(cli.fd, &efd);

		ret = select(nfds, &rfd, &wfd, &efd, &tv);
		if (ret < 0)
		{
			LOG("!!! select()\n");
			exit(1);
		}
		if (FD_ISSET(cli.in, &efd))
		{
			LOG("!!! select() on in: %s\n",
					strerror(errno));
			exit(1);
		}
		if (FD_ISSET(cli.fd, &efd))
		{
			LOG("!!! select() on fd: %s\n",
					strerror(errno));
			exit(1);
		}

		if (FD_ISSET(cli.in, &rfd))
			process_cli_data();
		if (FD_ISSET(cli.fd, &rfd))
			process_dect_data();

		if( (cli.hop) &&
				( (cli.mode & MODE_FPSCAN) ||
				  (cli.mode & MODE_PPSCAN) ||
				  (cli.mode & MODE_CALLSCAN) ||
				  (cli.mode & MODE_JAM   ) ))
		{
			if ( time(NULL) > cli.last_hop + cli.hop_ch_time )
			{
				cli.channel++;

				if (cli.channel > cli.hop_end)
					cli.channel = cli.hop_start;

				set_channel(cli.channel);
			}
		}

		if (cli.autorec)
		{
			if ( (time (NULL) - cli.autorec_last_bfield
			      > cli.autorec_timeout)
			    &&
			      (cli.mode != MODE_CALLSCAN)
			   )
			{
				do_stop_keep_autorec();
				do_callscan();
				if (cli.pcap)
				{
					pcap_dump_close(cli.pcap_d);
					pcap_close(cli.pcap);
					cli.pcap_d = NULL;
					cli.pcap   = NULL;
					cli.hop = 1;
				}
			}
		}
	}

}

int main(int argc, char ** argv)
{
	init();
	/* make stdout unbuffered */
	setvbuf(stdout,(char*)NULL,_IONBF,0);
	printf("DECT command line interface\n");
	printf("type \"help\" if you're lost\n");
	mainloop();
	return 0;
}
