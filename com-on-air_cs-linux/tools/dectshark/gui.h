#include <ncurses.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/select.h>

#include "foundinfo.h"
#include "config.h"

#if !defined(GUI_H)
#define GUI_H 

#define SW 80
#define SH 25

extern found_dects founds;
extern config cfg;


class print_gui
{
public:
	print_gui();
	~print_gui();

	void work();

	void up();
	void down();

	int getselected();


	void setchannel(int channel);
protected:
	void InitCurses();
	void RefreshScreen();
	void PrintFounds(int start,int num,int selected);
	void PrintStatus(unsigned int PP,unsigned int FP,int channel);
	int kbhit_wait(int time);

	pthread_mutex_t mutex;

	int selected,startpos,displaypos;

	WINDOW *mainwin;
	WINDOW *statuswin;
	WINDOW *msgwin;

	int channel;
};

#endif
