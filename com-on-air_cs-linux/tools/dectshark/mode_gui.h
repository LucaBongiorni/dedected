#if !defined(MODE_GUI_H)
#define MODE_GUI_H 

#include "dectshark.h"

class mode_gui
{
public:
	mode_gui()					{printf("ARRRR1!\n");}
	mode_gui(int x,int y)				{printf("ARRRR2!\n");}
	virtual ~mode_gui()				{printf("ARRRR3!\n");}

	virtual WINDOW *getmainwin()			{printf("ARRRR4!\n");return NULL;}

	virtual void tick()				{printf("ARRRR5!\n");}
	virtual unsigned int keypressed(int key)	{printf("ARRRR6!\n");return 0;}

	virtual void refreshscreen()			{printf("ARRRR7!\n");}

protected:

};

#endif
