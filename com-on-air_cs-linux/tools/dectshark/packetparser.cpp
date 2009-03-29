#include "packetparser.h"

#define DECT_A_TA		0xe0
#define DECT_A_Q1		0x10
#define DECT_A_BA		0x0e
#define DECT_A_Q2		0x01

#define DECT_Q_TYPE		0x80
#define DECT_N_TYPE		0x60
#define DECT_P_TYPE		0xe0


packetparser::packetparser()
{
	int i;

	for(i=0;i<24;i++)
	{
		syncinfo.slot[i].channel=0;
		syncinfo.slot[i].afields=0;
		syncinfo.slot[i].bfields=0;
		syncinfo.slot[i].berrors=0;
		syncinfo.slot[i].lastrssi=0;
	}
}

packetparser::~packetparser()
{
}

void packetparser::parsepacket(sniffed_packet packet)
{
	unsigned int slot=packet.slot;
	if(slot<24)
	{
		syncinfo.slot[slot].afields++;

		if(bfieldactive(packet))
		{
			syncinfo.slot[slot].bfields++;

			if(!bfieldok(packet))
				syncinfo.slot[slot].berrors++;
		}

		syncinfo.slot[slot].channel=packet.channel;
		syncinfo.slot[slot].lastrssi=packet.rssi;

		processrfpi(packet);

	}
}
	
slotinfo_str packetparser::getslotinfo(unsigned int slot)
{
	return syncinfo.slot[slot];
}

int packetparser::bfieldactive(sniffed_packet packet)
{
	if ((packet.data[5] & 0x0e) != 0x0e)
		return 1;
	return 0;
}

int packetparser::bfieldok(sniffed_packet packet)
{
	if(packet.frameflags&0xf0)
		return 1;

	return 0;
}

void packetparser::processrfpi(sniffed_packet packet)
{
/*
	if ((packet.data[5] & DECT_A_TA) == DECT_N_TYPE)
		return 1;

	return 0;*/
}


