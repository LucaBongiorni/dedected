#include "gui.h"

print_gui::print_gui()
{
}

print_gui::~print_gui()
{
	delete gui;
	endwin();
}


void print_gui::work()
{
	initcurses();
	while(1)
	{		
		if(kbhit_wait(300000))
		{
			int key=wgetch(gui->getmainwin());

			key=gui->keypressed(key);

			switch(key)
			{
				case 'q':
					cfg.stop();
					exit(0);
					break;	
			}
		}

		gui->tick();

		if(cfg.shouldrestart())
		{
			while(cfg.shouldstop())
				usleep(100000);

			delete gui;
			cfg.setchannel(cfg.getwantchannel());
			cfg.restarted();
			
			switch(cfg.getsniffmode())
			{
				case SNIFFMODE_SCAN:
					break;
				case SNIFFMODE_SYNC:
					gui=new syncmode_gui(COLS>=80?COLS:80,LINES>=25?LINES:25);
					break;
			}
		}
	}

}



void print_gui::initcurses()
{
	initscr();

	start_color();
	init_pair(1,COLOR_GREEN,COLOR_BLACK);
	init_pair(2,COLOR_YELLOW,COLOR_BLACK);
	init_pair(3,COLOR_WHITE,COLOR_BLACK);
	init_pair(4,COLOR_RED,COLOR_BLACK);
	init_pair(5,COLOR_YELLOW,COLOR_GREEN);
	init_pair(6,COLOR_RED,COLOR_GREEN);

	

	gui=new scanmode_gui(COLS>=80?COLS:80,LINES>=25?LINES:25);
}

void print_gui::refreshscreen()
{
	gui->refreshscreen();
}



int print_gui::kbhit_wait(int time)
{
	struct timeval tv;
	fd_set read_fd;

	tv.tv_sec=0;
	tv.tv_usec=time;

	FD_ZERO(&read_fd);
	FD_SET(0,&read_fd);

	if(select(1,&read_fd,NULL,NULL,&tv)==-1)
		return 0;

	if(FD_ISSET(0,&read_fd))
		return 1;

	return 0;
}
