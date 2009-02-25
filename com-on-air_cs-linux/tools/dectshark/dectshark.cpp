#include "dectshark.h"
#include "gui.h"


config cfg;
print_gui gui;


int main(int argc, char *argv[])
{
	if ( argc > 1 ) 
	{
		if ( strcmp( argv[1], "--help" ) == 0 ) 
		{
			printf("Usage: %s [--fp|--pp]\n", argv[0]);
			printf("  --fp    Scan Fixed Part (DECT Basestation)\n");
			printf("  --pp    Scan Portable Part (DECT Handset)\n");
			printf("  --help  This text\n");
			printf("%s without any parameter scans for Fixed Parts by default.\n", argv[0]);
			printf("\n");
			return 0;
		}

		if ( strcmp( argv[1], "--fp" ) == 0 ) 
		{
			cfg.setscanmode(SCANMODE_FP);
		}

		if ( strcmp( argv[1], "--pp" ) == 0 )
		{
			cfg.setscanmode(SCANMODE_PP);
		}
	}

	gui.work();

	return 0;
}






