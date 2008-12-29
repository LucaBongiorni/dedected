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

#define DEV "/dev/coa"

void *pcap_threadmain(void *threadid);
void *show_threadmain(void *threadid);


found_dects founds;
print_gui	gui;

pthread_t pcap_thread;
pthread_t show_thread;



void set_channel(int dev,int channel);

int main(int argc, char *argv[])
{
	int ch;

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
	val = COA_MODE_SNIFF|COA_SUBMODE_SNIFF_SCANPP;
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
			founds.AddDect(found);
		}
		
		usleep(100000);
		channeltime++;
	
		if(channeltime == 5)
		{
			chn++;
			chn %= 10;
			set_channel(dev,chn);
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





