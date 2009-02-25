#if !defined(SCANMODE_GUI_H)
#define SCANMODE_GUI_H

#include "dectshark.h"
#include "foundinfo.h"
#include "mode_gui.h"

extern config cfg;
void *scanthread(void *threadid);

class scanmode_gui : public mode_gui
{
public:
	scanmode_gui(int x,int y);
	~scanmode_gui();

	WINDOW *getmainwin();

	void tick();
	unsigned int keypressed(int key);

	void refreshscreen();

protected:
	void printfounds();
	void printstatus(unsigned int PP,unsigned int FP);
	
	void up();
	void down();


	pthread_mutex_t mutex;

	unsigned int selected,startpos,displaypos,scrw,scrh;

	WINDOW *mainwin;
	WINDOW *statuswin;
	WINDOW *msgwin;

	pthread_t workthread;

};

#endif
