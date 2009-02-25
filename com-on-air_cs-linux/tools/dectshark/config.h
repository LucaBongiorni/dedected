#if !defined(CONFIG_H)
#define CONFIG_H 

#define DEV "/dev/coa"

//TODO: make threadsafe

#define SNIFFMODE_SCAN	0
#define SNIFFMODE_SYNC	1

#define SCANMODE_FP	0
#define SCANMODE_PP	1

#define SYNC_SEARCH	0
#define SYNC_FOUND	1
#define SYNC_SYNCED	2

struct sharkconfig
{
	bool hop;
	int channel;

	int scanmode;
	int sniffmode;

	int wantchannel;
	unsigned char rfpi[5];

	int sync;

	int stop;
	int restart;
};

class config
{
public:
	config()				{cfg.hop=1;cfg.channel=0;cfg.scanmode=0;cfg.stop=0;cfg.restart=0;}
	~config()				{}

	void sethop(bool hop)			{cfg.hop=hop;}
	bool hop()				{return cfg.hop;}
	
	void setchannel(int chn)		{cfg.channel=chn;}
	int getchannel()			{return cfg.channel;}
	void channelinc()			{cfg.channel=(cfg.channel+1)%10;}

	void setscanmode(int mode)		{cfg.scanmode=mode;cfg.sniffmode=SNIFFMODE_SCAN;}
	void setsyncmode()			{cfg.sniffmode=SNIFFMODE_SYNC;}
	int getscanmode()			{return cfg.scanmode;}
	int getsniffmode()			{return cfg.sniffmode;}
	
	void setrfpi(unsigned char *rfpi)	{memcpy(cfg.rfpi,rfpi,5);}
	unsigned char *getrfpi()		{return (unsigned char*)&cfg.rfpi;}

	void setwantchannel(int chn)		{cfg.wantchannel=chn;}
	int getwantchannel()			{return cfg.wantchannel;}


	void setsync(int sync)			{cfg.sync=sync;}
	int getsync()				{return cfg.sync;}

	
	void restart()				{cfg.restart=1;}
	void restarted()			{cfg.restart=0;}
	int shouldrestart()			{return cfg.restart;}

	void stop()				{cfg.stop=1;}
	void stopped()				{cfg.stop=0;}
	int shouldstop()			{return cfg.stop;};
private:

	sharkconfig cfg;
};






#endif
