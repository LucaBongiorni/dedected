#if !defined(GUI_H)
#define GUI_H 

#include "dectshark.h"
#include "mode_gui.h"
#include "scanmode_gui.h"
#include "syncmode_gui.h"

#define SW 80
#define SH 25


class print_gui
{
public:
	print_gui();
	~print_gui();

	void work();

protected:
	void initcurses();
	void refreshscreen();

	int kbhit_wait(int time);

	pthread_mutex_t mutex;

	mode_gui *gui;

};

#endif
