#if !defined(PACKETSAVER_H)
#define PACKETSAVER_H 

#include "dectshark.h"

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

class packetsaver
{
public:
	packetsaver();
	~packetsaver();

	int openfilerfpi(unsigned char *RFPI);
	int openfile(char *fn);
	void closefile();

	void savepacket(sniffed_packet packet);
	
protected:
	char errbuf[PCAP_ERRBUF_SIZE];

	pcap_t		*pcap;
	pcap_dumper_t	*pcap_d;
};




#endif
