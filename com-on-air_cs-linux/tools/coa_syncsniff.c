/*
 * coa_syncsniff dumps pcap files on a given channel and RFPI
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

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>


#include <sys/socket.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <netinet/ether.h>

#include "com_on_air_user.h"


struct sniffed_packet
{
	unsigned char     rssi;
	unsigned char     channel;
	unsigned char     slot;
	struct timespec   timestamp;
	unsigned char     data[53];
};

struct pcap_global_header
{ 
	uint32_t magic_number;   /* magic number */
	uint16_t version_major;  /* major version number */
	uint16_t version_minor;  /* minor version number */
	int   	 thiszone;       /* GMT to local correction */
	uint32_t sigfigs;        /* accuracy of timestamps */
	uint32_t snaplen;        /* max length of captured packets, in octets */
	uint32_t network;        /* data link type */
};


struct pcap_record_header
{ 
	uint32_t ts_sec;         /* timestamp seconds */
	uint32_t ts_usec;        /* timestamp microseconds */
	uint32_t incl_len;       /* number of octets of packet saved in file */
	uint32_t orig_len;       /* actual length of packet */
};

void write_global_header(FILE *pcap);
void write_record(FILE *pcap,uint32_t sec,uint32_t usec,uint32_t len,unsigned char *record);

#define DEV "/dev/coa"

/* we use some "hidden" ioctls */
#define COA_IOCTL_TEST0 0xF000
#define COA_IOCTL_TEST1 0xF001
#define COA_IOCTL_TEST2 0xF002
#define COA_IOCTL_TEST3 0xF003
#define COA_IOCTL_TEST4 0xF004
#define COA_IOCTL_TEST5 0xF005
#define COA_IOCTL_TEST6 0xF006
#define COA_IOCTL_TEST7 0xF007


#define COA_IOCTL_DUMP_DIP_RAM 	COA_IOCTL_TEST0
#define COA_IOCTL_FIFO_TEST     COA_IOCTL_TEST1
#define COA_IOCTL_COUNT_IRQ	COA_IOCTL_TEST2

/* default RFPI */
uint8_t RFPI[5]={0x00,0x00,0x00,0x00,0x00};


int main(int argc, char *argv[])
{
	int d;
	int ret = 0;

	FILE *pcap;
	if(argc<2)
	{
		printf(	"Usage:coa_syncsniff channel pcap-file [RFPI]\n");
		exit(-1);	
	}

	d=open(DEV, O_RDONLY);
	if (d<0)
	{
		printf("couldn't open(\"%s\"): %s\n", DEV, strerror(errno));
		exit(1);
	}

        pcap=fopen(argv[2],"wb");
        if(!pcap)
        {
		printf("Cant open pcap file for write...\n");
		exit(1);
	}

	/* optionally accept RFPI as 3rd argument on commandline */
	if(argc>2)
	{
		sscanf(argv[3], "%hhx %hhx %hhx %hhx %hhx", &RFPI[0], &RFPI[1], &RFPI[2], &RFPI[3], &RFPI[4]);
		printf("RFPI: %02x %02x %02x %02x %02x\n", RFPI[0], RFPI[1], RFPI[2], RFPI[3], RFPI[4]);
	}


	//set sync sniff mode
	uint16_t val;
	val = COA_MODE_SNIFF|COA_SUBMODE_SNIFF_SYNC;
	if(ioctl(d,COA_IOCTL_MODE, &val)){printf("couldn't ioctl()\n");exit(1);}

	//set rfpi to sync with
	if(ioctl(d,COA_IOCTL_SETRFPI, RFPI)){printf("couldn't ioctl()\n");exit(1);}

	//set channel
	uint32_t chn=atoi(argv[1]);
	printf("set channel %u\n",chn);
	if(ioctl(d,COA_IOCTL_CHAN,&chn)){printf("couldn't set channel\n");exit(1);}


	write_global_header(pcap);

	//sniff-loop
        while (0xDEC + 't')
	{
		struct sniffed_packet buf;
	        while (sizeof(struct sniffed_packet) == (ret = read(d, &buf, (sizeof(struct sniffed_packet)))))
		{
	        	unsigned char packet[100];
			packet[12]=0x23;
			packet[13]=0x23;
			packet[14]=0x00;		//decttype (receive)
			packet[15]=buf.channel;		//channel
			packet[16]=0;
			packet[17]=buf.slot;		//slot
			packet[18]=0;
			packet[19]=buf.rssi;
			memcpy(packet+20,buf.data,53);

			write_record(
				pcap,
				buf.timestamp.tv_sec,
				buf.timestamp.tv_nsec/1000,
				73,
				packet);
		}
	}





	return ret;
}


void write_global_header(FILE *pcap)
{
	struct pcap_global_header header;

	header.magic_number=0xa1b2c3d4;
	header.version_major=2;
	header.version_minor=4;
	header.thiszone=0;//GMT
	header.sigfigs=0;
	header.snaplen=1024;
	header.network=1;

	fwrite(&header,1,sizeof(struct pcap_global_header),pcap);
}

void write_record(FILE *pcap,uint32_t sec,uint32_t usec,uint32_t len,unsigned char *record)
{
	struct pcap_record_header header;
	header.ts_sec=sec;
	header.ts_usec=usec;
	header.incl_len=len;
	header.orig_len=len;

	fwrite(&header,1,sizeof(struct pcap_record_header),pcap);
	fwrite(record,1,len,pcap);
}


