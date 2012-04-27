#ifndef AUDIODECODE_H
#define AUDIODECODE_H

#include <stdint.h>
#include "dect_cli.h"

extern char imaDumping;
extern char wavDumping;
extern struct cli_info cli;

struct wavHeader
{
	char chunkID[4];
	unsigned int chunkSize;
	char format[4];
	
	// Fmt subchunk
	char subChunk1ID[4];
	unsigned int subchunk1Size;
	unsigned short audioFormat;
	unsigned short numChannels;
	unsigned int sampleRate;
	unsigned int byteRate;
	unsigned short blockAlign;
	unsigned short bitsPerSample;

	// Data subchunk
	char subchunk2ID[4];
	unsigned int subchunk2Size;
};





void initWavHeader(struct wavHeader *header);

char openIma(char *filename);

char closeIma();

char openWav(char *filename);

char closeWav();

char openAlsa();

char closeAlsa();

char queueSample(short sample);

char packetAudioProcessing(uint8_t *pcap_packet);

void *play();


#endif
