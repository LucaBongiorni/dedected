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
#include <fcntl.h>

#include "com_on_air_user.h"


#include "foundinfo.h"
#include "gui.h"
#include "config.h"


#define DEV "/dev/coa"


void *pcap_threadmain(void *threadid);
void *show_threadmain(void *threadid);

config cfg;


found_dects founds;
print_gui	gui;

pthread_t pcap_thread;
pthread_t show_thread;



void set_channel(int dev,int channel);

int scan_type = 0;
int main(int argc, char *argv[])
{
	int ch;
	if ( argc > 1 ) {
		if ( strcmp( argv[1], "--help" ) == 0 ) {
			printf("Usage: %s [--fp|--pp]\n", argv[0]);
			printf("  --fp    Scan Fixed Part (DECT Basestation)\n");
			printf("  --pp    Scan Portable Part (DECT Handset)\n");
			printf("  --help  This text\n");
			printf("%s without any parameter scans for Fixed Parts by default.\n", argv[0]);
			printf("\n");
			return 0;
		}
		if ( strcmp( argv[1], "--fp" ) == 0 ) {
			scan_type = 0;
		}
		if ( strcmp( argv[1], "--pp" ) == 0 ) {
			scan_type = 1;
		}
	}

	pthread_create(&pcap_thread, NULL, pcap_threadmain, (void *)0);
//	pthread_create(&show_thread, NULL, show_threadmain, (void *)0);

//	while(1);

	gui.work();

	return 0;
}





void *pcap_threadmain(void *threadid)
{
	int tid;
	int chn=0,channeltime=0;
  	int dev;

        dev = open(DEV, O_RDONLY);
	if (dev<0){
		printf("couldn't open(\"%s\"): %s\n", DEV, strerror(errno));
	}

        uint16_t val;
	if ( scan_type == 0 ) {
		val = COA_MODE_SNIFF|COA_SUBMODE_SNIFF_SCANFP; // scan fixed part
	} else { 
		val = COA_MODE_SNIFF|COA_SUBMODE_SNIFF_SCANPP; // scan portable part
	}

	if (ioctl(dev, COA_IOCTL_MODE, &val)){
		printf("couldn't set sniff mode\n");
	}


	while(0xDEC + 't')		// ;)
	{
		dect_found found;
		
		unsigned char buf[7];
		while (7 == (read(dev, buf, 7)))
		{
			memcpy(found.RFPI,buf+2,5);
			found.channel=buf[0];
			found.type=DECT_FOUND_FP;
			found.rssi=buf[1];
			founds.AddDect(found);
		}
		
		usleep(100000);
		channeltime++;
	
		if(channeltime == 5)
		{
			if(cfg.hop())
			{
				chn++;
				chn %= 10;
				set_channel(dev,chn);
			}
			channeltime = 0;
		}
	}

   pthread_exit(NULL);
}

void set_channel(int dev,int channel)
{
	if (ioctl(dev, COA_IOCTL_CHAN, &channel)){
		printf("couldn't set channel\n");
	}

	gui.setchannel(channel);
}



void *show_threadmain(void *threadid)
{
	gui.work();
	pthread_exit(NULL);
}





