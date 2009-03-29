#if !defined(DECTSHARK_GUI_H)
#define DECTSHARK_GUI_H 

#include <ncurses.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/select.h>

#include <ncurses.h>
#include <unistd.h>
#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <fcntl.h>
#include <pcap.h>

#include "com_on_air_user.h"

#include "config.h"

extern config cfg;



void printnil(char *,...);

#if 0
#define LOG printf
#else
#define LOG printnil
#endif


struct sniffed_packet
{
	unsigned char rssi;
	unsigned char channel;
	unsigned char slot;
	unsigned char frameflags;
	struct timespec   timestamp;
	unsigned char data[53];
};


#endif
