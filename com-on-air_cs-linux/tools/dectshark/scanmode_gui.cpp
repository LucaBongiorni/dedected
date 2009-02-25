#include "scanmode_gui.h"

found_dects	founds;

scanmode_gui::scanmode_gui(int x,int y)
{
	selected=startpos=displaypos=0;

	scrw=x;
	scrh=y;	

	mainwin         = newwin(scrh,scrw-20,0,0);
	statuswin       = newwin(scrh,20,0,scrw-20);

	noecho();
	keypad(mainwin,TRUE);

	wbkgdset(mainwin,(COLOR_PAIR(1)|A_BOLD));
	wbkgdset(statuswin,(COLOR_PAIR(1)|A_BOLD));


	box(mainwin,0,0);
	wattron(mainwin,COLOR_PAIR(3));
	mvwprintw(mainwin,1,1,"RFPI");
	mvwprintw(mainwin,1,13,"Ch");
	mvwprintw(mainwin,1,34,"Pkt");
	mvwprintw(mainwin,1,53,"RSSI");
	wnoutrefresh(mainwin);

	box(statuswin,0,0);
	wattron(statuswin,COLOR_PAIR(2));
	mvwprintw(statuswin,1,1,"Founds:");
	mvwprintw(statuswin,4,1,"Packets:");
	mvwprintw(statuswin,scrh-3,1,"Channel:");
	wnoutrefresh(statuswin);


	doupdate();

	pthread_create(&workthread, NULL, scanthread, (void *)0);


}

scanmode_gui::~scanmode_gui()
{
}

WINDOW *scanmode_gui::getmainwin()
{
	return mainwin;
}



unsigned int scanmode_gui::keypressed(int key)
{

	if((key>'0')&&(key<'9'))
	{
		cfg.stop();
		cfg.setwantchannel(key-0x30);
		cfg.restart();
		return 0;
	}

	switch(key)
	{
		case KEY_LEFT:
			break;
		case KEY_RIGHT:
			break;
		case KEY_UP:
			up();
			break;
		case KEY_DOWN:
			down();
			break;
		case 'h':
			cfg.sethop(!cfg.hop());
			break;
		case 's':
			{
				dect_listentry entry=founds.GetListEntry(selected);
				if(entry.valid)
				{				
					cfg.stop();
					cfg.setrfpi(entry.RFPI);
					cfg.setwantchannel(entry.channel);
					cfg.setsyncmode();
					cfg.restart();
					return 0;
				}
			}
			break;
		default:
			return key;
	}

	return 0;
}

void scanmode_gui::tick()
{
	pthread_mutex_lock(&mutex);

	printfounds();
	printstatus(startpos,founds.GetListLength());

	pthread_mutex_unlock(&mutex);
}


void scanmode_gui::refreshscreen()
{
	wnoutrefresh(mainwin);
	wnoutrefresh(statuswin);

	doupdate();
}


void scanmode_gui::printfounds()
{
	unsigned int i,y;

	y=2;
	for(i=startpos;i<(startpos+scrh-3);i++)
	{
		dect_listentry entry=founds.GetListEntry(i);

		if(i==selected)
		{
			if(entry.RFPI[4]&0x07)
				wattron(mainwin,COLOR_PAIR(6));
			else
				wattron(mainwin,COLOR_PAIR(5));
		}		
		else
		{
			if(entry.RFPI[4]&0x07)
				wattron(mainwin,COLOR_PAIR(4));
			else
				wattron(mainwin,COLOR_PAIR(2));
		}

		if(entry.valid)
		{
			mvwprintw(mainwin,y,1,"%.2x%.2x%.2x%.2x%.2x  ",entry.RFPI[0],entry.RFPI[1],entry.RFPI[2],entry.RFPI[3],entry.RFPI[4]);
			mvwprintw(mainwin,y,13,"%.2i          %12u        %12u",entry.channel,entry.fppackets,entry.rssi);
			y++;
		}
	}

	wrefresh(mainwin);
}


void scanmode_gui::printstatus(unsigned int PP,unsigned int FP)
{
	wattron(statuswin,COLOR_PAIR(3));
	mvwprintw(statuswin,2,1,"%17u",FP);
	mvwprintw(statuswin,5,1,"%17u",PP);

	mvwprintw(statuswin,scrh-2,16,"%2u",cfg.getchannel());
	wrefresh(statuswin);
}

void scanmode_gui::up()
{
	pthread_mutex_lock(&mutex);

	if(founds.GetListLength()<=(scrh-3))
	{
		if(displaypos>0)
		{
			displaypos--;
			selected--;
		}
	}
	else
	{
		if(displaypos>0)
		{
			displaypos--;
			selected--;
		}
		else if(startpos>0)
		{
			selected--;
			startpos--;
		}
	}

	pthread_mutex_unlock(&mutex);
}

void scanmode_gui::down()
{
	pthread_mutex_lock(&mutex);

	if(founds.GetListLength()<=(scrh-3))
	{
		if(displaypos<(founds.GetListLength()-1))
		{
			displaypos++;
			selected++;
		}
	}
	else
	{
		if(displaypos<(scrh-4))
		{
			displaypos++;
			selected++;
		}
		else if(startpos<=(founds.GetListLength()-1-(scrh-3)))
		{
			selected++;
			startpos++;
		}

	}

	pthread_mutex_unlock(&mutex);
}


/**********************************
 ****      worker thread       ****
 *********************************/

void *scanthread(void *threadid)
{
	int channeltime=0;
  	int dev;

        dev = open(DEV, O_RDONLY);
	if (dev<0)
	{
		printf("couldn't open(\"%s\"): %s\n", DEV, strerror(errno));
	}

        uint16_t val;
	if (cfg.getscanmode() == SCANMODE_FP) 
	{
		val = COA_MODE_SNIFF|COA_SUBMODE_SNIFF_SCANFP; // scan fixed part
	}
	else 
	{ 
		val = COA_MODE_SNIFF|COA_SUBMODE_SNIFF_SCANPP; // scan portable part
	}

	if (ioctl(dev, COA_IOCTL_MODE, &val))
	{
		printf("couldn't set sniff mode\n");
	}


	while(0xDEC + 'T')		// ;)
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
		if(cfg.shouldstop())
		{
			close(dev);
			cfg.stopped();
			pthread_exit(NULL);
		}


		channeltime++;
	
		if(channeltime == 5)
		{
			if(cfg.hop())
			{
				cfg.channelinc();

				int channel=cfg.getchannel();	
				if (ioctl(dev, COA_IOCTL_CHAN, &channel))
					printf("couldn't set channel\n");
			}
			channeltime = 0;
		}
	}

}



