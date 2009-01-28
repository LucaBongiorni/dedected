#if !defined(CONFIG_H)
#define CONFIG_H 

//TODO: make threadsafe

struct sharkconfig
{
	bool hop;
	int mode;
};

class config
{
public:
	config()	{cfg.hop=1;}
	~config()	{}

	void sethop(bool hop)	{cfg.hop=hop;}
	bool hop()		{;return cfg.hop;}

private:

	sharkconfig cfg;
};






#endif
