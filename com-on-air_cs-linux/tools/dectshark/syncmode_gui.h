#if !defined(SYNCMODE_GUI_H)
#define SYNCMODE_GUI_H 

#include "dectshark.h"
#include "mode_gui.h"
#include "packetparser.h"
#include "packetsaver.h"

void *syncthread(void *threadid);

class syncmode_gui: public mode_gui
{
public:
	syncmode_gui(int x,int y);
	~syncmode_gui();

	WINDOW *getmainwin();

	void tick();
	unsigned int keypressed(int key);

	void refreshscreen();

protected:
	void printfounds();
	void printstatus(unsigned int PP,unsigned int FP);
	

	pthread_mutex_t mutex;

	unsigned int selx,sely,scrw,scrh;

	WINDOW *mainwin;
	WINDOW *statuswin;
	WINDOW *msgwin;
	
	WINDOW *popupwin;

	pthread_t workthread;

};

#endif
