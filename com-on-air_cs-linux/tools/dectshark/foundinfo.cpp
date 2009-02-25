#include "foundinfo.h"

found_dects::found_dects()
{
	pthread_mutex_init(&mutex,NULL);

	listlen=0;
	list=(dect_listentry*)malloc(0);
}

found_dects::~found_dects()
{
	listlen=0;
	free(list);

	pthread_mutex_destroy(&mutex);
}


void found_dects::AddDect(dect_found found)
{
	unsigned int i;

	pthread_mutex_lock(&mutex);

	for(i=0;i<listlen;i++)
	{
		if(list[i].channel==found.channel)
		{
			if(!memcmp(list[i].RFPI,found.RFPI,5))
			{
				if(found.type==DECT_FOUND_FP)
					list[i].fppackets++;
				else
					list[i].pppackets++;

				list[i].rssi=found.rssi;

				pthread_mutex_unlock(&mutex);
				return;
			}
		}
	}

	listlen++;
	list=(dect_listentry*)realloc(list,listlen*sizeof(dect_listentry));
	
	memcpy(list[listlen-1].RFPI,found.RFPI,5);
	list[listlen-1].channel=found.channel;
	list[listlen-1].rssi=found.rssi;

	pthread_mutex_unlock(&mutex);


}


void found_dects::ClearList()
{
	pthread_mutex_lock(&mutex);

	list=(dect_listentry*)realloc(list,0);
	listlen=0;

	pthread_mutex_unlock(&mutex);
}

dect_listentry found_dects::GetListEntry(unsigned int entry)
{
	pthread_mutex_lock(&mutex);

	if(entry<listlen)
	{
		dect_listentry temp=list[entry];
		temp.valid=true;

		pthread_mutex_unlock(&mutex);
		return temp;
	}
	else
	{
		pthread_mutex_unlock(&mutex);

		dect_listentry temp;
		temp.valid=false;

		return temp;
	}
}

unsigned int found_dects::GetListLength()
{
	int tlen;

	pthread_mutex_lock(&mutex);
	tlen=listlen;
	pthread_mutex_unlock(&mutex);

	return tlen;
}
