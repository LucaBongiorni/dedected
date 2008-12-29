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
#include <pcap.h>


#include "com_on_air_user.h"
#include "dect_cli.h"

//#define DUMP_IRQ_COUNT_ONCE_PER_SEC

struct cli_info cli;


#define  RXBUF 8192
char buf[RXBUF];


/* pcap errors */
char errbuf[PCAP_ERRBUF_SIZE];

void print_help(void)
{
	LOG("\n");
	LOG("   help          - this help\n");
	LOG("   fpscan        - async scan for basestations, dump RFPIs\n");
	LOG("   callscan      - async scan for active calls, dump RFPIs\n");
	LOG("   autorec       - sync on any calls in callscan, autodump in pcap, currently %s\n", cli.autorec ? "ON":"OFF");
	LOG("   ppscan <rfpi> - sync scan for active calls\n");
	LOG("   chan <ch>     - set current channel [0-9], currently %d\n", cli.channel);
//	LOG("   slot <sl>     - set current slot [0-23], currently %d\n", cli.slot);
//	LOG("   jam           - jam current channel\n");
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
		LOG("### switching to channel %d\n", channel);
	if (ioctl(cli.fd, COA_IOCTL_CHAN, &channel)){
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
	if (cli.autorec)
		do_ppscan(station->RFPI);
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

void do_ppscan_str(char * str_rfpi)
{
	uint8_t RFPI[5];
	char * end;
	int i;
	for (i=0; i<5; i++)
	{
		RFPI[i] = strtoul(str_rfpi, &end, 16);
		if ((errno == ERANGE )
			|| (errno != 0 && RFPI[i] == 0))
		{
			LOG("!!! please enter a valid RFPI (e.g. 00 01 02 03 04)\n");
			return;
		}
		if (end == str_rfpi)
		{
			LOG("!!! please enter a valid RFPI (e.g. 00 01 02 03 04)\n");
			return;
		}
		str_rfpi = end;
	}
	do_ppscan(RFPI);
}

void do_chan(char * str_chan)
{
	uint32_t channel;
	char * end;
	channel = strtoul(str_chan, &end, 0);
	if ((errno == ERANGE && (channel == LONG_MAX || channel == LONG_MIN))
			|| (errno != 0 && channel == 0))
	{
		LOG("!!! please enter a valid channel number [0-9]\n");
		return;
	}
	if (end == str_chan)
	{
		LOG("!!! please enter a valid channel number [0-9]\n");
		return;
	}
	if (channel > 9)
	{
		LOG("!!! please enter a valid channel number [0-9]\n");
		return;
	}
	cli.channel = channel;
	set_channel(cli.channel);
}

void do_slot(char * str_chan)
{
	uint32_t slot;
	char * end;
	slot = strtoul(str_chan, &end, 0);
	if ((errno == ERANGE && (slot == LONG_MAX || slot == LONG_MIN))
			|| (errno != 0 && slot == 0))
	{
		LOG("!!! please enter a valid slot number [0-23]\n");
		return;
	}
	if (end == str_chan)
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

void do_dump(void)
{
	int i;
	struct dect_station * p = cli.station_list;
	if (!p)
	{
		LOG("### nothing found so far\n");
		return;
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
	LOG("### autorec turned %s\n", cli.autorec ? "ON":"OFF");
}

void do_stop(void)
{
	if (!(cli.mode & MODE_STOP))
	{
		LOG("### stopping DIP\n");
		uint16_t val;
		val = COA_MODE_IDLE;
		if (ioctl(cli.fd, COA_IOCTL_MODE, &val)){
			LOG("couldn't ioctl()\n");
			exit(1);
		}
		cli.mode = MODE_STOP;
		cli.autorec = 0;
	}
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
	if ( !strncasecmp((char *)buf, "dump", 4) )
		{ do_dump(); done = 1; }
	if ( !strncasecmp((char *)buf, "hop", 3) )
		{ do_hop(); done = 1; }
	if ( !strncasecmp((char *)buf, "verb", 4) )
		{ do_verb(); done = 1; }
	if ( !strncasecmp((char *)buf, "stop", 4) )
		{ do_stop(); done = 1; }
	if ( !strncasecmp((char *)buf, "quit", 4) )
		{ do_stop(); exit(0); }

	if(!done)
		LOG("!!! no such command %s\n", buf);

}

void init_pcap(struct sniffed_packet * packet)
{
	char fname[100];
	sprintf(fname, "dump_%.2x_%.2x_%.2x_%.2x_%.2x.pcap",
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
	if ((cli.packet.data[0x19] & 0x0e) != 0x0e)
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
				pcap_packet[16] = 0;
				pcap_packet[17] = cli.packet.slot;
				pcap_packet[18] = 0;
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

	cli.autorec             = 0;
	cli.autorec_timeout     = 10;
	cli.autorec_last_bfield = 0;
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

#ifdef DUMP_IRQ_COUNT_ONCE_PER_SEC
#define COA_IOCTL_COUNT_IRQ 0xF002
	uint32_t lasttime = time(NULL);
#endif


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

#ifdef DUMP_IRQ_COUNT_ONCE_PER_SEC
		if (!(cli.mode & MODE_STOP))
			if (time(NULL) >= lasttime + 1)
			{
				if (ioctl(cli.fd, COA_IOCTL_COUNT_IRQ, NULL))
				{
					printf("couldn't ioctl()\n");
					exit(1);
				}
				lasttime = time(NULL);
			}
#endif

		if( (cli.hop) &&
				( (cli.mode & MODE_FPSCAN) || 
				  (cli.mode & MODE_CALLSCAN) ||
				  (cli.mode & MODE_JAM   ) ))
		{
			if ( time(NULL) > cli.last_hop + cli.hop_ch_time )
			{
				cli.channel++;
				cli.channel %= 10;
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
				do_stop();
				do_callscan();
				if (cli.pcap)
				{
					pcap_dump_close(cli.pcap_d);
					pcap_close(cli.pcap);
					cli.pcap_d = NULL;
					cli.pcap   = NULL;
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
