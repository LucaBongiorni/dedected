#if !defined(MODE_GUI_H)
#define MODE_GUI_H 

#include "dectshark.h"


class mode_gui
{
public:
	mode_gui()					{LOG("ARRRR1!\n");}
	mode_gui(int x,int y)				{LOG("ARRRR2!\n");}
	virtual ~mode_gui()				{LOG("ARRRR3!\n");}

	virtual WINDOW *getmainwin()			{LOG("ARRRR4!\n");return NULL;}

	virtual void tick()				{LOG("ARRRR5!\n");}
	virtual unsigned int keypressed(int key)	{LOG("ARRRR6!\n");return 0;}

	virtual void refreshscreen()			{LOG("ARRRR7!\n");}

protected:

};

#endif
