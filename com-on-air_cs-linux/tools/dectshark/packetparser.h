#if !defined(PACKETPARSER_H)
#define PACKETPARSER_H 

#include "dectshark.h"

struct sniffed_packet
{
	unsigned char rssi;
	unsigned char channel;
	unsigned char slot;
	unsigned char framenumber;
//	unsigned char bfok;
	struct timespec   timestamp;
	unsigned char data[53];
};


struct slotinfo_str
{
	unsigned int channel;
	unsigned int afields;
	unsigned int bfields;
	unsigned int berrors;
	unsigned int lastrssi;
	unsigned char rfpi[5][5];
};

struct syncinfo_str
{
	slotinfo_str slot[24];
};


class packetparser
{
public:
	packetparser();
	~packetparser();

	void parsepacket(sniffed_packet packet);
	
	slotinfo_str getslotinfo(unsigned int slot);

protected:
	syncinfo_str syncinfo;

	int bfieldactive(sniffed_packet packet);
	void processrfpi(sniffed_packet packet);
};




#endif
