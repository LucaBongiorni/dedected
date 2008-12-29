/*
 * dect_cli async and sync interface to DECT, can dump pcap files
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * authors:
 * (C) 2008  Andreas Schuler <krater at badterrorist dot com>
 * (C) 2008  Matthias Wenzel <dect at mazzoo dot de>
 *
 */

#ifndef DECT_CLI_H
#define DECT_CLI_H

#define DEV "/dev/coa"

// too verbose #define LOG(fmt, args...) printf("%s(): " fmt, __FUNCTION__, ##args)
#define LOG(fmt, args...) printf(fmt, ##args)


#define TYPE_FP 23
#define TYPE_PP 42

struct dect_station
{
	struct dect_station   * next;
	uint8_t               RFPI[5];
	uint32_t              RSSI;
	uint8_t               channel;
	uint8_t               type;
	uint32_t              first_seen;
	uint32_t              last_seen;
	uint32_t              count_seen;
};

struct sniffed_packet
{
	unsigned char rssi;
	unsigned char channel;
	unsigned char slot;
	struct timespec   timestamp;
	unsigned char data[53];
};


#define MODE_STOP     0x00000001
#define MODE_FPSCAN   0x00000002
#define MODE_PPSCAN   0x00000004
#define MODE_CALLSCAN 0x00000008
#define MODE_JAM      0x00000010


struct cli_info
{
	uint8_t               RFPI[5];
	uint32_t              channel;
	uint32_t              slot;
	int                   hop;
	int                   hop_ch_time; /* in sec */
	uint32_t              last_hop;

	uint32_t              mode;

	int                   in; /* stdin */
	int                   fd; /* filehandle to the DECT-trx */

	int                   verbose;
	int                   autorec;
	int                   autorec_timeout;
	int                   autorec_last_bfield;

	/* fpscan (async) list of stations */
	struct dect_station   station;
	struct dect_station   * station_list;

	/* ppscan (sync) */
	struct sniffed_packet packet;
	pcap_t                * pcap;
	pcap_dumper_t         * pcap_d;
};


#endif /* DECT_CLI_H */
