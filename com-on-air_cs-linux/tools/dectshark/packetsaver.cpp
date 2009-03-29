#include "packetsaver.h"


packetsaver::packetsaver()
{
	pcap=NULL;
}

packetsaver::~packetsaver()
{
	closefile();
}

int packetsaver::openfilerfpi(unsigned char *RFPI)
{
	char fn[512];
	char ftime[256];
	time_t rawtime;
	struct tm *timeinfo;

	time (&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(ftime, sizeof(ftime), "%Y-%m-%d_%H_%M_%S", timeinfo);

	sprintf(fn, "dump_%s_RFPI_%.2x_%.2x_%.2x_%.2x_%.2x.pcap",ftime,
			RFPI[0],RFPI[1],RFPI[2],RFPI[3],RFPI[4]);

	LOG("### dumping to %s\n", fn);

	return openfile(fn);
}

int packetsaver::openfile(char *fn)
{
	pcap = pcap_open_dead(DLT_EN10MB, 74);
	if (!pcap)
	{
		LOG("!!! couldn't pcap_open_dead(\"%s\")\n", fn);
		return 0;
	}

	pcap_d = pcap_dump_open(pcap, fn);
	if (!pcap_d)
	{
		LOG("!!! couldn't pcap_dump_open(\"%s\")\n", fn);
		return 0;
	}

	return 1;
}

void packetsaver::closefile()
{
	if (pcap)
	{
		pcap_dump_close(pcap_d);
		pcap_close(pcap);
		pcap_d = NULL;
		pcap   = NULL;
	}
}

void packetsaver::savepacket(sniffed_packet packet)
{
	struct pcap_pkthdr pcap_hdr;
	int ret,length;

	if(pcap_d)
	{
		if ((packet.data[5] & 0x0e) != 0x0e)
			length = 74;
		else
			length = 33;

		pcap_hdr.caplen = length;
		pcap_hdr.len = length;
		ret = gettimeofday(&pcap_hdr.ts, NULL);
		if (ret)
		{
			LOG("!!! couldn't gettimeofday(): %s\n",
			strerror(errno));
			exit(1);
		}

		uint8_t pcap_packet[74];
		memset(pcap_packet, 0, 74);
		pcap_packet[12] = 0x23;
		pcap_packet[13] = 0x23;
		pcap_packet[14] = 0x00;        /* decttype (receive) */
		pcap_packet[15] = packet.channel;
		pcap_packet[16] = 0x00;
		pcap_packet[17] = packet.slot;
		pcap_packet[18] = packet.frameflags&0x0f;
		pcap_packet[19] = packet.rssi;
		memcpy(&pcap_packet[20], packet.data, 53);
		pcap_packet[73] = 0x00;

		pcap_dump((u_char*)pcap_d, &pcap_hdr, pcap_packet);

	}
}
