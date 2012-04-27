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
	char*                 name;
};

struct sniffed_packet
{
	unsigned char rssi;
	unsigned char channel;
	unsigned char slot;
	unsigned char frameflags;
	struct timespec   timestamp;
	unsigned char data[53];
};


struct rfpi_list
{
	struct rfpi_list * next;
	uint8_t RFPI[5];
};

struct station_name
{
	struct station_name * next;
	struct station_name * prev;
	uint8_t RFPI[5];
	char* name;
};

#define MODE_STOP     0x00000001
#define MODE_FPSCAN   0x00000002
#define MODE_PPSCAN   0x00000004
#define MODE_CALLSCAN 0x00000008
#define MODE_JAM      0x00000010

#define DECT_BAND_EMEA 0x01
#define DECT_BAND_US   0x02

struct cli_info
{
	uint8_t               RFPI[5];
	uint32_t              channel;
	uint32_t              slot;
	int                   hop;
	int                   hop_ch_time; /* in sec */
	uint32_t              last_hop;

	uint8_t               band;      /* DECT_BAND_EMEA, DECT_BAND_US       */
	uint8_t               hop_start; /* 0 for DECT/EMEA, 10 for US/DECT6.0 */
	uint8_t               hop_end;   /* 9 for DECT/EMEA, 14 for US/DECT6.0 */

	uint32_t              mode;

	int                   in; /* stdin */
	int                   fd; /* filehandle to the DECT-trx */

	int                   verbose;
	int                   autorec;
	int                   autorec_timeout;
	int                   autorec_last_bfield;

	int                   imaDump;
	int                   wavDump;
	int                   imaDumping;
	int                   wavDumping;
	int                   audioPlay;
	int                   audioPlaying;
	int                   channelPlaying;

	/* fpscan (async) list of stations */
	struct dect_station   station;
	struct dect_station   * station_list;

	struct station_name * station_names;

	/* ignored RFPIs */
	struct rfpi_list * ignored_rfpis;

	/* ppscan (sync) */
	struct sniffed_packet packet;
	pcap_t                * pcap;
	pcap_dumper_t         * pcap_d;
};


#endif /* DECT_CLI_H */
