/*
 * pcap2cchan dumps C-channel information from pcap files
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * authors:
 * (C) 2008  Andreas Schuler <krater at badterrorist dot com>
 * (C) 2008  Matthias Wenzel <dect at mazzoo dot de>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dect_c_channel.h"



int main(int argc, char* argv[])
{
	struct cfrag frag;
	struct cpacket cpacket;

	struct cdevice phone,station;
	phone.type=station.type=0;
	phone.cdata=station.cdata=0;
	phone.found=station.found=0;
	phone.cnt=station.cnt=0;

	unsigned char packet[135];
	unsigned int length;

	FILE *pcap=fopen(argv[1],"rb");

	fseek(pcap,0x18,SEEK_SET);

	while(!feof(pcap))
	{
		fseek(pcap,12,SEEK_CUR);
		fread(&length,4,1,pcap);
		
		if(length>135)
		{
			printf("error , packet size too big\n");
			exit(-1);
		}

		fread(packet,length,1,pcap);
		frag=getcfrag(packet+23,packet[17]);

		if(frag.valid)
		{
			if(packet[23]==0x16)
			{
				cpacket=getcpacket(frag,&phone);

				//printf("Frag   : %u  ->  %u:%.2x %.2x %.2x %.2x %.2x\n",frag.slot,frag.cttype,frag.data[0],frag.data[1],frag.data[2],frag.data[3],frag.data[4]);

				if(cpacket.valid)
				{
					printcpacket(cpacket,"phone  ");
				}
			}
			else if(packet[23]==0xe9)
			{
				cpacket=getcpacket(frag,&station);

				//printf("Frag   : %u  ->  %u:%.2x %.2x %.2x %.2x %.2x\n",frag.slot,frag.cttype,frag.data[0],frag.data[1],frag.data[2],frag.data[3],frag.data[4]);


				if(cpacket.valid)
					printcpacket(cpacket,"station");
			}

		}

	}

	fclose(pcap);
	return 0;
}


struct cfrag getcfrag(unsigned char *dect,int slot)
{
	struct cfrag frag;

	if((dect[2]&0xc0)==0)			//Ct tail (?)
	{
		frag.valid=1;

		frag.cttype=(dect[2]&0x20)>>5;
		frag.slot=slot;
		memcpy(frag.data,dect+3,5);

	}
	else
		frag.valid=0;

	return frag;
}


char *getstring(struct strtype *str,int id,int max)
{
	int i;
	for(i=0;i<max;i++)
	{
		if(str[i].type==id)
			return str[i].name;
	}

	return NULL;
}



struct cpacket getcpacket(struct cfrag frag,struct cdevice *device)
{
	device->packet.valid=0;

	if((frag.cttype!=device->type)&&(!device->found))
	{
		device->packet.addr=frag.data[0];
		device->packet.ctrl=frag.data[1];
		device->packet.length=frag.data[2];

		if((device->packet.length>>2)>0)
		{
			device->packet.data[0]=frag.data[3];
			device->packet.data[1]=frag.data[4];
			device->cdata+=2;
			device->cnt=1;
			device->found=1;
		}
		else
		{
			device->packet.checksum=(((unsigned short)(frag.data[3]))<<8)|(frag.data[4]);
			device->packet.valid=1;
			device->cdata=0;
			device->cnt=0;
			device->found=0;
		}
		
		device->type=!device->type;

	}
	else if(frag.cttype!=device->type)
	{
		memcpy(device->packet.data+device->cdata,frag.data,5);
		device->cdata+=5;
		device->cnt++;
		device->type=!device->type;
		
		if(device->cnt>=pklookup[device->packet.length>>2])
		{
			device->packet.checksum=(((unsigned short)(frag.data[3]))<<8)|(frag.data[4]);
			device->packet.valid=1;
			device->cdata=0;
			device->cnt=0;
			device->found=0;
		}

	}
	else
	{
		//printf("packet loss (?)\n");
		device->type=!device->type;

		if(device->found)
		{
			memset(device->packet.data+device->cdata,0xff,5);
			device->cdata+=5;
			device->cnt++;

			if(device->cnt>=pklookup[device->packet.length>>2])
			{
				device->cdata=0;
				device->packet.valid=1;
				device->found=0;

				printcpacket(device->packet,"failed ");
			}
		}


		getcpacket(frag,device);

	}
	

	return device->packet;
}





void printcpacket(struct cpacket packet,char *prefix)
{
	int x;

	printf("\n%s: addr:%.2x ctrl:%.2x len:%.2x crc:%.4x",prefix,packet.addr,packet.ctrl,packet.length,packet.checksum);

	if(packet.length>>2)
	{
		printf(" -> ");

		char id=packet.data[1];
		switch(packet.data[0]&0x0f)
		{
			case 0:
				printf("%s :%s",msgtype[packet.data[0]&0x0f].name,NULL);
				break;
			case 3:
				printf("%s :%s",msgtype[packet.data[0]&0x0f].name,getstring(cctype,id,256));
				break;
			case 4:
				printf("%s :%s",msgtype[packet.data[0]&0x0f].name,getstring(sstype,id,256));
				break;
			case 5:
				printf("%s :%s",msgtype[packet.data[0]&0x0f].name,getstring(mmtype,id,256));
				break;
			case 6:
				printf("%s :%s",msgtype[packet.data[0]&0x0f].name,NULL);
				break;
			case 7:
				printf("%s :%s",msgtype[packet.data[0]&0x0f].name,NULL);
				break;

			default:
				printf("reserved ");
				break;
		}

		printf("   ");

		for(x=0;x<(packet.length>>2);x++)
			printf(" %.2x",packet.data[x]);
	}

//	printf("\n\n\n");
}
