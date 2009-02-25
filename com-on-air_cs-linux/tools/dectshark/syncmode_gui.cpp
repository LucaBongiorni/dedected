#include "syncmode_gui.h"


packetparser pparser;



syncmode_gui::syncmode_gui(int x,int y)
{
	int i;

	selx=sely=0;

	scrw=x;
	scrh=y;	

	mainwin         = newwin(scrh-4,scrw-20,0,0);
	statuswin       = newwin(scrh,20,0,scrw-20);
	msgwin          = newwin(4,scrw-20,scrh-4,0);

	noecho();

	wbkgdset(mainwin,(COLOR_PAIR(1)|A_BOLD));
	wbkgdset(statuswin,(COLOR_PAIR(1)|A_BOLD));
	wbkgdset(msgwin,(COLOR_PAIR(1)|A_BOLD));


	box(mainwin,0,0);
	wattron(mainwin,COLOR_PAIR(3));
	mvwprintw(mainwin,1,1,"Slot");
	mvwprintw(mainwin,1,6,"Ch");
	mvwprintw(mainwin,1,22,"FP");
	mvwprintw(mainwin,1,46,"PP");

	wattron(mainwin,COLOR_PAIR(2));
	for(i=0;i<25;i+=24)
	{
		mvwprintw(mainwin,2,15+i,"A");
		mvwprintw(mainwin,2,22+i,"B");
		mvwprintw(mainwin,2,25+i,"Err");
		mvwprintw(mainwin,2,31+i,"R");
	}

	wnoutrefresh(mainwin);

	box(statuswin,0,0);
	wattron(statuswin,COLOR_PAIR(2));
	mvwprintw(statuswin,1,1,"Founds:");
	mvwprintw(statuswin,4,1,"Packets:");
	mvwprintw(statuswin,scrh-3,1,"Channel:");
	wnoutrefresh(statuswin);

	box(msgwin,0,0);
	wnoutrefresh(msgwin);


	//starting search
	cfg.setsync(SYNC_SEARCH);

	popupwin = newwin(3,42,(scrh/2)-2,(scrw/2)-21);
	keypad(popupwin,TRUE);
	wbkgdset(popupwin,(COLOR_PAIR(4)|A_BOLD));
	box(popupwin,0,0);
	wattron(popupwin,COLOR_PAIR(2));
	unsigned char *RFPI=cfg.getrfpi();
	mvwprintw(popupwin,1,1,"  Trying to sync to RFPI %.2x%.2x%.2x%.2x%.2x ... ",RFPI[0],RFPI[1],RFPI[2],RFPI[3],RFPI[4]);
	wnoutrefresh(popupwin);


	doupdate();

	pthread_create(&workthread, NULL, syncthread, (void *)0);


}

syncmode_gui::~syncmode_gui()
{
	delwin(mainwin);
	delwin(statuswin);
	delwin(msgwin);
}

WINDOW *syncmode_gui::getmainwin()
{
	return mainwin;
}



unsigned int syncmode_gui::keypressed(int key)
{
	if(cfg.getsync()==SYNC_SYNCED)
	{
		switch(key)
		{
			case KEY_LEFT:
				selx=0;
				break;
			case KEY_RIGHT:
				selx=1;
				break;
			case KEY_UP:
				if(sely>0)
					sely--;
				break;
			case KEY_DOWN:
				if(sely<11)
					sely++;
				break;
			default:
				return key;
		}
	
		printfounds();
	}
	else
	{
		return key;
	}

	return 0;
}

void syncmode_gui::tick()
{
	pthread_mutex_lock(&mutex);

	switch(cfg.getsync())
	{
		case SYNC_SYNCED:
			printfounds();
			printstatus(selx,sely);
			break;
		case SYNC_FOUND:
			delwin(popupwin);

			wbkgdset(mainwin,(COLOR_PAIR(1)|A_BOLD));
			wbkgdset(statuswin,(COLOR_PAIR(1)|A_BOLD));
			wbkgdset(msgwin,(COLOR_PAIR(1)|A_BOLD));

			box(mainwin,0,0);
			box(statuswin,0,0);
			box(msgwin,0,0);

			keypad(mainwin,TRUE);

			doupdate();
			cfg.setsync(SYNC_SYNCED);
			break;
		case SYNC_SEARCH:
			break;
	}

	pthread_mutex_unlock(&mutex);
}


void syncmode_gui::refreshscreen()
{
	if(cfg.getsync()==SYNC_SYNCED)
	{
		wnoutrefresh(mainwin);
		wnoutrefresh(statuswin);
		wnoutrefresh(msgwin);
	}
	else
	{
		wnoutrefresh(popupwin);
	}

	doupdate();
}


void syncmode_gui::printfounds()
{
	int i;

	slotinfo_str slotinfo;

	wattron(mainwin,COLOR_PAIR(1));
	for(i=0;i<12;i++)
	{
		slotinfo=pparser.getslotinfo(i);

		wattron(mainwin,COLOR_PAIR(1));
		mvwprintw(mainwin,i+3,1,"%2u   %.2u",i,slotinfo.channel);

		if((sely==i)&&(selx==0))
			wattron(mainwin,COLOR_PAIR(5));
		else
			wattron(mainwin,COLOR_PAIR(1));

		mvwprintw(mainwin,i+3,8,"%8u%7u%5u  %2u",slotinfo.afields,slotinfo.bfields,slotinfo.berrors,slotinfo.lastrssi);



		slotinfo=pparser.getslotinfo(i+12);

		if((sely==i)&&(selx==1))
			wattron(mainwin,COLOR_PAIR(5));
		else
			wattron(mainwin,COLOR_PAIR(1));

		mvwprintw(mainwin,i+3,32,"%8u%7u%5u  %2u",slotinfo.afields,slotinfo.bfields,slotinfo.berrors,slotinfo.lastrssi);
	}

	wrefresh(mainwin);
}


void syncmode_gui::printstatus(unsigned int PP,unsigned int FP)
{
	wattron(statuswin,COLOR_PAIR(3));
	mvwprintw(statuswin,2,1,"%17u",FP);
	mvwprintw(statuswin,5,1,"%17u",PP);

	mvwprintw(statuswin,scrh-2,16,"%2u",cfg.getchannel());
	wrefresh(statuswin);
}




/**********************************
 ****      worker thread       ****
 *********************************/

void process_dect_data(int dev);

void *syncthread(void *threadid)
{
	fd_set rfd;
	fd_set efd;

	struct timeval tv;

	int ret;

  	int dev;

        dev = open(DEV, O_RDONLY);
	if (dev<0)
		printf("couldn't open(\"%s\"): %s\n", DEV, strerror(errno));

	uint16_t val;

	val=COA_MODE_SNIFF|COA_SUBMODE_SNIFF_SYNC;
	if (ioctl(dev, COA_IOCTL_MODE, &val))
		printf("couldn't set mode()\n");

	unsigned char *RFPI=cfg.getrfpi();
	if(ioctl(dev, COA_IOCTL_SETRFPI, RFPI))
		printf("couldn't set rfpi\n");

	int channel=cfg.getchannel();
	if (ioctl(dev, COA_IOCTL_CHAN, &channel))
		printf("couldn't set channel\n");


	while (0xDEC + 'T')
	{
		tv.tv_sec  = 1;
		tv.tv_usec = 0;

		FD_ZERO(&rfd);
		FD_ZERO(&efd);

		FD_SET(dev, &rfd);
		FD_SET(dev, &efd);

		ret = select(dev+1, &rfd, NULL, &efd, &tv);
		if (ret < 0)
			printf("select() read error\n");

		if (FD_ISSET(dev, &efd))
			printf("select() exception\n");

		if (FD_ISSET(dev, &rfd))
		{
			if(cfg.getsync()==SYNC_SEARCH)
				cfg.setsync(SYNC_FOUND);

			process_dect_data(dev);
		}

	}



}


void process_dect_data(int dev)
{
	sniffed_packet packet;

	while ( sizeof(packet) == read(dev, &packet, sizeof(packet)))
	{
		pparser.parsepacket(packet);
	}
}




