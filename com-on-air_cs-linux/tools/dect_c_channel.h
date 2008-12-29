/*
 * DECT C channel definitions
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

#ifndef DECT_CCHAN_H
#define DECT_CCHAN_H


struct strtype
{
	char type;
	char name[100];
};

struct strtype msgtype[16]=
{
	{0,"0000 LCE  (Link Control Entity) messages"},
	{1,""},
	{2,""},
	{3,"0011 CC   (Call Control) messages"},
	{4,"0100 CISS (Call Independent Supplementary Services) messages"},
	{5,"0101 MM   (Mobility Management) messages"},
	{6,"0110 CLMS (ConnectionLess Message Service) messages"},
	{7,"0111 COMS (Connection Oriented Message Service) messages"}
};

struct strtype cctype[256]=
{
	{0x01,"{CC-ALERTING}"},
	{0x02,"{CC-CALL-PROC}"},
	{0x05,"{CC-SETUP}"},
	{0x07,"{CC-CONNECT}"},
	{0x0d,"{CC-SETUP-ACK}"},
	{0x0f,"{CC-CONNECT-ACK}"},
	{0x20,"{CC-SERVICE-CHANGE}"},
	{0x21,"{CC-SERVICE-ACCEPT}"},
	{0x23,"{CC-SERVICE-REJECT}"},
	{0x4d,"{CC-RELEASE}"},
	{0x5a,"{CC-RELEASE-COM}"},
	{0x60,"{IWU-INFO}"},
	{0x6e,"{CC-NOTIFY}"},
	{0x7b,"{CC-INFO}"}
};

struct strtype mmtype[256]=
{
	{0x40,"{AUTHENTICATION-REQUEST}"},
	{0x41,"{AUTHENTICATION-REPLY}"},
	{0x42,"{KEY-ALLOCATE}"},
	{0x43,"{AUTHENTICATION-REJECT}"},
	{0x44,"{ACCESS-RIGHTS-REQUEST}"},
	{0x45,"{ACCESS-RIGHTS-ACCEPT}"},
	{0x47,"{ACCESS-RIGHTS-REJECT}"},
	{0x48,"{ACCESS-RIGHTS-TERMINATE-REQUEST}"},
	{0x49,"{ACCESS-RIGHTS-TERMINATE-ACCEPT}"},
	{0x4b,"{ACCESS-RIGHTS-TERMINATE-REJECT}"},
	{0x4c,"{CIPHER-REQUEST}"},
	{0x4e,"{CIPHER-SUGGEST}"},
	{0x4f,"{CIPHER-REJECT}"},
	{0x50,"{MM-INFO-REQUEST}"},
	{0x51,"{MM-INFO-ACCEPT}"},
	{0x52,"{MM-INFO-SUGGEST}"},
	{0x53,"{MM-INFO-REJECT}"},
	{0x54,"{LOCATE-REQUEST}"},
	{0x55,"{LOCATE-ACCEPT}"},
	{0x56,"{DETACH}"},
	{0x57,"{LOCATE-REJECT}"},
	{0x58,"{IDENTITY-REQUEST}"},
	{0x5a,"{IDENTITY-REPLY}"},
	{0x5b,"{MM-IWU}"},
	{0x5c,"{TEMPORARY-IDENTITY-ASSIGN}"},
	{0x5d,"{TEMPORARY-IDENTITY-ASSIGN-ACK}"},
	{0x5f,"{TEMPORARY-IDENTITY-ASSIGN-REJ}"},
	{0x6e,"{MM-NOTIFY}"}
};

struct strtype sstype[256]=
{
	{0x24,"{HOLD}"},
	{0x28,"{HOLD-ACK}"},
	{0x30,"{HOLD-REJECT}"},
	{0x31,"{RETRIEVE}"},
	{0x33,"{RETRIEVE-ACK}"},
	{0x37,"{RETRIEVE-REJECT}"},
	{0x5a,"{CISS-RELEASE-COM}"},
	{0x62,"{FACILITY}"},
	{0x64,"{CISS-REGISTER}"},
};


struct cfrag
{
	int valid;
	int slot;
	char cttype;
	unsigned char data[5];
};

struct cpacket
{
	int valid;
	unsigned char addr;
	unsigned char ctrl;
	unsigned char length;
	unsigned char data[80];
	unsigned short checksum;
};

struct cdevice
{
	char type;
	int found;
	char cnt;
	int cdata;
	struct cpacket packet;
};

char lenlookup[64]={
5,10,10,10,10,10,15,15,
15,15,15,20,20,20,20,20,
25,25,25,25,25,30,30,30,
30,30,35,35,35,35,35,40,
40,40,40,40,45,45,45,45,
45,50,50,50,50,50,55,55,
55,55,55,60,60,60,60,60,
65,65,65,65,65,70,70,70
};

char pklookup[64]={
1,2,2,2,2,2,3,3,
3,3,3,4,4,4,4,4,
5,5,5,5,5,6,6,6,
6,6,7,7,7,7,7,8,
8,8,8,8,9,9,9,9,
9,10,10,10,10,10,11,11,
11,11,11,12,12,12,12,12,
13,13,13,13,13,14,14,14
};


struct cfrag   getcfrag(unsigned char *dect,int slot);
struct cpacket getcpacket(struct cfrag frag,struct cdevice *device);

void printcpacket(struct cpacket packet,char *prefix);
char *getstring(struct strtype *str,int id,int max);



#endif
