/*
 * Audio dumping and playing support for the dect_cli tool
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * authors:
 * (C) 2008  Ramiro Pareja <ramiropareja at gmail dot com>
 *
 * based on the work of:
 * (C) 2008  Matthias Wenzel <dect at mazzoo dot de>
 * (C) 2008  Andreas Schuler <krater at badterrorist dot com>
 *
 * NOTES:
 *   - Only G721 support by now. G726 support very soon!
 *   - Sound quality is not perfect. The G721 library has a flaw in the decoding
 *     algorithm when the output is a PCM stream.
 *     Will be soon solved using another library or using A-law compression in the
 *     WAV file.
 *
 */


#include <pcap.h>
#include <alsa/asoundlib.h>
#include <pthread.h>
#include <sched.h>
#include <semaphore.h>

#include "pcapstein.h"
#include "codec/g72x.h"
#include "audioDecode.h"

#define ALSA_PCM_NEW_HW_PARAMS_API	//Use the new ALSA API


FILE *fpImaFP, *fpImaPP, *fpWavFP, *fpWavPP;

int pp_slot;
int fp_slot;

int numSamplesFP, numSamplesPP;

short *audioBuffer;
unsigned short bp = 0;
snd_pcm_t *handle;
snd_pcm_hw_params_t *params;
snd_pcm_uframes_t frames = 640; 
unsigned int rate = 8000;
int dir;



pthread_t playingThread;
pthread_attr_t tattr;
pthread_attr_t tattrMain;
unsigned int bufferLength;
sem_t semaphore;

struct g72x_state stateFP, statePP;

// Header of a 8KHz, 16bits, mono, PCM encoded Wav file
struct wavHeader wavHeaderDefault = 
{
	{'R','I','F','F'}, 	//chunkID
	0,			//chunkSize
	{'W','A','V','E'}, 	//format
	{'f','m','t',' '}, 	//subChunk1ID
	16, 			//subchunk1Size
	1, 			//audioFormat
	1, 			//numChannels
	8000, 			//sampleRate
	16000, 			//byteRate
 	2, 			//blockAlign
	16, 			//bitsPerSample
	{'d','a','t','a'}, 	//subchunk2ID
	0			//subchunk2Size
};

/******************************************************************************
* OpenIma: Create the IMA files                                               *
******************************************************************************/

char openIma(char *filename)
{
	char tmp[200];
	
	pp_slot = -1;
	fp_slot = -1;

	fpImaFP = fpImaPP = NULL;

	// Trying to create the ima files. 
	sprintf(tmp,"%.190s_FP.ima",filename);
	fpImaFP = fopen(tmp,"w");	

	sprintf(tmp,"%.190s_PP.ima",filename);
	fpImaPP = fopen(tmp,"w");	


	if (fpImaPP != NULL && fpImaFP != NULL)
	{		
		printf("### Dumping audio in IMA format\n");

		cli.imaDumping = 1;
		return 0;
	}
	else
	{
		printf("### Error creating IMA files\n");

		cli.imaDumping = 0;
		return 1;
	}
}



/******************************************************************************
* CloseIma: Close the IMA files                                               *
******************************************************************************/

char closeIma()
{
	fclose(fpImaFP);
	fclose(fpImaPP);

	printf("### Closing IMA files\n");

	cli.imaDumping = 0;
		
	return 0;
}


/******************************************************************************
* OpenWav: Create the WAV files                                               *
******************************************************************************/

char openWav(char *filename)
{
	char tmp[200];

	pp_slot = -1;
	fp_slot = -1;

	fpWavFP = fpWavPP = NULL;

	numSamplesFP = numSamplesPP = 0;

	g72x_init_state(&stateFP);
	g72x_init_state(&statePP);


	// Trying to create the wav files. 
	sprintf(tmp,"%.190s_FP.wav",filename);
	fpWavFP = fopen(tmp,"w");
	
	sprintf(tmp,"%.190s_PP.wav",filename);
	fpWavPP = fopen(tmp,"w");		

	if (fpWavPP != NULL && fpWavFP != NULL)
	{

		// Insert the WAV header	
		fwrite(&wavHeaderDefault,sizeof(wavHeaderDefault),1, fpWavFP);
		fwrite(&wavHeaderDefault,sizeof(wavHeaderDefault),1, fpWavPP);

		printf("### Dumping audio in WAV format\n");

		cli.wavDumping = 1;
		return 0;
	}
	else
	{
		printf("### Error creating WAV files\n");

		cli.wavDumping = 0;
		return 1;
	}
}


/******************************************************************************
* closeWav: Close the WAV files                                               *
******************************************************************************/

char closeWav()
{
	unsigned int chunkSize, chunk2Size;

	
	
	// Update the WAV headers with the number of samples

	chunk2Size = 2 * numSamplesFP;
	chunkSize = chunk2Size + 36;
	fseek(fpWavFP,4,SEEK_SET);
	fwrite(&chunkSize,sizeof(unsigned int),1,fpWavFP);
	fseek(fpWavFP,40,SEEK_SET);
	fwrite(&chunk2Size,sizeof(unsigned int),1,fpWavFP);

	chunk2Size = 2 * numSamplesPP;
	chunkSize = chunk2Size + 36;
	fseek(fpWavPP,4,SEEK_SET);
	fwrite(&chunkSize,sizeof(unsigned int),1,fpWavPP);
	fseek(fpWavPP,40,SEEK_SET);
	fwrite(&chunk2Size,sizeof(unsigned int),1,fpWavPP);


	fclose(fpWavFP);
	fclose(fpWavPP);

	cli.wavDumping = 0;

	printf("### Closing WAV files\n");
	return 0;
}


/******************************************************************************
* openAlsa: Open the ALSA device and create a thread                          *
******************************************************************************/

char openAlsa()
{

	pp_slot = -1;
	fp_slot = -1;

	struct sched_param paramMain;
	struct sched_param paramThread;

	// Open PCM for playback
	if ( snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0)
	{	
		printf("### Error opening the ALSA device\n");

		cli.audioPlay = 0;
		return 1;
	}
	
	// Allocating the hardware parameters and filling with defaults
	snd_pcm_hw_params_alloca(&params);
	snd_pcm_hw_params_any(handle, params);
	
	// Configure the hardware 
	snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED); //Interleaved
	snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE); //PCM linear, 16 bits, signed
	snd_pcm_hw_params_set_channels(handle, params, 1); //Mono
	snd_pcm_hw_params_set_rate_near(handle, params, &rate, &dir); //8Khz sampling rate	
	snd_pcm_hw_params_set_period_size_near(handle, params, &frames, &dir); // period size: 80 frames
	
	// Update the new hardware configuration	
	if (snd_pcm_hw_params(handle, params) < 0)
	{
		printf("### Error configuring the ALSA device\n");

  		snd_pcm_close(handle);

		cli.audioPlay = 0;
		return 1;
	}

	// Recover the "real" period size and allocate memory for one period
	snd_pcm_hw_params_get_period_size(params, &frames, &dir);
	bufferLength = frames * 1000;
	audioBuffer = (short *) malloc(bufferLength * sizeof(short));
	bp = 0;


	cli.audioPlaying = 1;
	sem_init(&semaphore, 0, 0);


	// Set the scheduler and the priority of the main thread
	paramMain.sched_priority = 60;
	if (sched_setscheduler(0,SCHED_RR,&paramMain)!= 0)
	{
		printf("### Unable to set the priority\n");
	}


	// Create the playing thread			
	pthread_attr_init(&tattr);
  	pthread_attr_setschedpolicy(&tattr, SCHED_RR);
	paramThread.sched_priority = 60;
	pthread_attr_setschedparam (&tattr, &paramThread);
	pthread_create( &playingThread, &tattr, &play, NULL);
	

	printf("### Opening ALSA device\n");

	return 0;
}


/******************************************************************************
* closeAlsa: Close the Alsa device and kill the thread                        *
******************************************************************************/

char closeAlsa()
{
	
	cli.audioPlaying = 0;
	
	// Open the semaphore to unlock the thread
	sem_post(&semaphore);

	// Wait for the thread exit
	pthread_join(playingThread, NULL);
	
	sem_destroy(&semaphore);

  	snd_pcm_drain(handle);
  	snd_pcm_close(handle);
  	free(audioBuffer);

	printf("### Closing ALSA device\n");

	return 0;

}

/******************************************************************************
* queueSample: push a sample in the playback buffer                           *
******************************************************************************/
char queueSample(short sample)
{

	static int p = 0;

	audioBuffer[p] = sample;

	bp++;
	p++;
	

	if (bp >= frames)
	{
		bp = 0;	
		sem_post(&semaphore);
	}

	if (p >= bufferLength)
	{
		p = 0;
	}	

	return 0;	
}


/******************************************************************************
* play: playback thread                                                       *
******************************************************************************/

void *play()
{

	static int p = 0;

	while (cli.audioPlaying)
	{
		
		sem_wait(&semaphore);
			

		// Play a period.	
		if (snd_pcm_writei(handle, &(audioBuffer[p]), frames) ==  -EPIPE)
		{
			// Underrun!!!
			snd_pcm_prepare(handle);		
		}
		
		p += frames;

		if (p >= bufferLength)
		{
			p = 0;		
		}
		
	}

	return;
}

/******************************************************************************
* packetAudioProccessing: process the audio packet                            *
******************************************************************************/

char packetAudioProcessing(uint8_t *pcap_packet)
{

	unsigned char code, data;
	unsigned char *p;
	short sample;
	int i;



	// Check if the packet has useful information

	if (pcap_packet[ETH_TYPE_0_OFF] != ETH_TYPE_0)
		return 1;
	if (pcap_packet[ETH_TYPE_1_OFF] != ETH_TYPE_1)
		return 1;
		
	if ((pcap_packet[PKT_OFF_H] & DECT_H_BA_MASK) == DECT_H_BA_NO_B_FIELD)
		return 1;
			
	if ((pcap_packet[PKT_OFF_H] & DECT_H_BA_MASK) == DECT_H_BA_HALF_SLOT)
	{
		fprintf(stderr, "unsopported half slot\n");
		return 1;
	}

	if ((pcap_packet[PKT_OFF_H] & DECT_H_BA_MASK) == DECT_H_BA_DOUBLE_SLOT)
	{
		fprintf(stderr, "unsopported double slot\n");
		return 1;
	}

	
	// Useful packet. Check if comes from the phone or the base station

	if ( (pcap_packet[0x17] == 0x16) && (pcap_packet[0x18] == 0x75) )
	{
		if (pp_slot < 0)
		{
			pp_slot = pcap_packet[0x11];
		}
		else
		{
			if (pp_slot == pcap_packet[0x11])
			{
				if (cli.imaDumping || cli.wavDumping || cli.audioPlaying)
				{
					p = &pcap_packet[0x21];
					
					// Each packet has 40 useful bytes.
					for (i = 0; i<40; i++)
					{	
						// Nibble swapping 
						data = (((*p)>>4) | ((*p)<<4));	
						p++;
					
						if (cli.imaDumping)
						{
							fwrite(&data,sizeof(unsigned char),1,fpImaPP);
						}
						

						if (cli.wavDumping || cli.audioPlaying)
						{
							// Processing the low nibble
							code = (data & 0x0F);
							sample = (short) g721_decoder(code, AUDIO_ENCODING_LINEAR, &statePP);
		
							if (cli.audioPlaying && (cli.channelPlaying == 0))
								queueSample(sample);
							
							if (cli.wavDumping)
								fwrite(&sample,sizeof(short),1,fpWavPP);

							numSamplesPP++;
							
							// Processing the high nibble
							code = (data >> 4);
							sample = (short) g721_decoder(code, AUDIO_ENCODING_LINEAR, &statePP);

							if (cli.audioPlaying && (cli.channelPlaying == 0))
								queueSample(sample);

							if (cli.wavDumping)
								fwrite(&sample,sizeof(short),1,fpWavPP);

							numSamplesPP++;							
						}
					}



				}
					
			}	
		}
	}
	else
	{
		if (fp_slot < 0)
		{
			fp_slot = pcap_packet[0x11];
		}
		else
		{
			if (fp_slot == pcap_packet[0x11])
			{

				if (cli.imaDumping || cli.wavDumping || cli.audioPlaying)
				{
					p = &pcap_packet[0x21];

					// Each packet has 40 useful bytes.
					for (i = 0; i<40; i++)
					{	
						// Nibble swapping 
						data = (((*p)>>4) | ((*p)<<4));	
						p++;

						if (cli.imaDumping)
						{
							fwrite(&data,sizeof(unsigned char),1,fpImaFP);
						}				
						
	
						if (cli.wavDumping || cli.audioPlaying)
						{
							// Processing the low nibble
							code = (data & 0x0F);
							sample = (short) g721_decoder(code, AUDIO_ENCODING_LINEAR, &stateFP);

							if (cli.audioPlaying && (cli.channelPlaying == 1))
								queueSample(sample);

							if (cli.wavDumping)
								fwrite(&sample,sizeof(short),1,fpWavFP);

							numSamplesFP++;
														
							// Processing the high nibble
							code = (data >> 4);
							sample = (short) g721_decoder(code, AUDIO_ENCODING_LINEAR, &stateFP);

							if (cli.audioPlaying && (cli.channelPlaying == 1))
								queueSample(sample);

							if (cli.wavDumping)
								fwrite(&sample,sizeof(short),1,fpWavFP);

							numSamplesFP++;
							
						}
					}

				}
			}	
		}
	}

	return 0;

}
