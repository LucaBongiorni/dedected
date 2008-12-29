#include "gui.h"

print_gui::print_gui()
{
	selected=startpos=displaypos=0;
}

print_gui::~print_gui()
{
	endwin();
}


void print_gui::up()
{
	pthread_mutex_lock(&mutex);

	if(founds.GetListLength()<=(SH-7))
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

void print_gui::down()
{
	pthread_mutex_lock(&mutex);

	if(founds.GetListLength()<=(SH-7))
	{
		if(displaypos<(founds.GetListLength()-1))
		{
			displaypos++;
			selected++;
		}
	}
	else
	{
		if(displaypos<(SH-8))
		{
			displaypos++;
			selected++;
		}
		else if(startpos<=(founds.GetListLength()-1-(SH-7)))
		{
			selected++;
			startpos++;
		}

	}

	pthread_mutex_unlock(&mutex);
}

int print_gui::getselected()
{
	pthread_mutex_lock(&mutex);
	return selected;
	pthread_mutex_unlock(&mutex);
}




void print_gui::work()
{
	InitCurses();

	while(1)
	{
		
		if(kbhit_wait(300000))
		{
			int ch=wgetch(mainwin);

			switch(ch)
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
				case 'l':
					//SetLockMode();
					break;
				case 'o':
					//SetOverviewMode();
					break;
			}
		}

		pthread_mutex_lock(&mutex);

		PrintFounds(startpos,SH-7,selected);
		PrintStatus(startpos,founds.GetListLength(),channel);


		pthread_mutex_unlock(&mutex);
	}

}



void print_gui::InitCurses()
{
	initscr();


	start_color();
	init_pair(1,COLOR_GREEN,COLOR_BLACK);
	init_pair(2,COLOR_YELLOW,COLOR_BLACK);
	init_pair(3,COLOR_WHITE,COLOR_BLACK);
	init_pair(4,COLOR_RED,COLOR_BLACK);
	init_pair(5,COLOR_YELLOW,COLOR_GREEN);

	mainwin         = newwin(SH-4,SW-20,0,0);
	statuswin       = newwin(SH,20,0,SW-20);
	msgwin          = newwin(4,SW-20,SH-4,0);

//	cbreak();
	noecho();
	keypad(mainwin,TRUE);

	wbkgdset(mainwin,(COLOR_PAIR(1)|A_BOLD));
	wbkgdset(statuswin,(COLOR_PAIR(1)|A_BOLD));
	wbkgdset(msgwin,(COLOR_PAIR(1)|A_BOLD));


	box(mainwin,0,0);
	wattron(mainwin,COLOR_PAIR(3));
	mvwprintw(mainwin,1,1,"RFPI");
	mvwprintw(mainwin,1,13,"Ch");
	mvwprintw(mainwin,1,31,"FP-Pkt");
	mvwprintw(mainwin,1,51,"PP-Pkt");
	wnoutrefresh(mainwin);

	box(statuswin,0,0);
	wattron(statuswin,COLOR_PAIR(2));
	mvwprintw(statuswin,1,1,"Station Packets:");
	mvwprintw(statuswin,4,1,"Phone Packets:");
	mvwprintw(statuswin,SH-3,1,"Channel:");
	wnoutrefresh(statuswin);

	box(msgwin,0,0);
	wnoutrefresh(msgwin);

	doupdate();

}

void print_gui::RefreshScreen()
{
	wnoutrefresh(mainwin);
	wnoutrefresh(statuswin);
	wnoutrefresh(msgwin);

	doupdate();
}


void print_gui::PrintFounds(int start,int num,int selected)
{
	int i,y;

	y=2;
	for(i=start;i<(start+num);i++)
	{
		if(i==selected)
			wattron(mainwin,COLOR_PAIR(5));
		else
			wattron(mainwin,COLOR_PAIR(2));

		dect_listentry entry=founds.GetListEntry(i);

		if(entry.valid)
		{
			mvwprintw(mainwin,y,1,"%.2x%.2x%.2x%.2x%.2x  ",entry.RFPI[0],entry.RFPI[1],entry.RFPI[2],entry.RFPI[3],entry.RFPI[4]);
			mvwprintw(mainwin,y,13,"%.2i          %12u        %12u",entry.channel,entry.fppackets,entry.pppackets);
			y++;
		}
	}

	wrefresh(mainwin);
}


void print_gui::PrintStatus(unsigned int PP,unsigned int FP,int channel)
{
	wattron(statuswin,COLOR_PAIR(3));
	mvwprintw(statuswin,2,1,"%17u",FP);
	mvwprintw(statuswin,5,1,"%17u",PP);

	mvwprintw(statuswin,SH-2,16,"%2u",channel);
	wrefresh(statuswin);
}

void print_gui::setchannel(int chn)
{
	pthread_mutex_lock(&mutex);
	channel=chn;
	pthread_mutex_unlock(&mutex);
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
