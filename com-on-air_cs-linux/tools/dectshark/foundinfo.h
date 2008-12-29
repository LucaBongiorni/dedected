#include <pthread.h>
#include <malloc.h>
#include <string.h>

#if !defined(FOUNDINFO_H)
#define FOUNDINFO_H 


#define DECT_FOUND_FP			0
#define DECT_FOUND_PP			1
#define DECT_FOUND_LISTENTRY		2
#define DECT_FOUND_INVALID		0xFFFF

struct dect_found
{
	unsigned int	type;
	unsigned char	RFPI[5];
	char		channel;	
};


struct dect_listentry
{
	unsigned char	RFPI[5];
	char		channel;
	unsigned int	fppackets;
	unsigned int	pppackets;
	bool		valid;
};



class found_dects
{
public:
	found_dects();
	~found_dects();

	void			AddDect(dect_found found);
	void			ClearList();
	dect_listentry		GetListEntry(unsigned int entry);
	int			GetListLength();


protected:

	int listlen;
	dect_listentry *list;

	pthread_mutex_t mutex;

};

#endif

