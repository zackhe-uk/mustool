/*
 * mustool - an all-in-one DMX audio library conversion tool
 * Copyleft Max Parry 2025 
 *
 * This is free software released into the public domain.
 *
 * mus.c - MUS file parser
 */
 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "mus.h"
#include "shared.h"

unsigned char* songData = NULL;

void mus_readHeader(FILE *fileP, musheader_t *musH)
{
	myfread(&musH->sig, sizeof(unsigned char), 4, fileP);

	if (verbose)
		printf("Signature: %.4s\n", musH->sig);

	if ( memcmp(musH->sig, mus_sig, 4) != 0 )
	{
		fprintf(stderr, "File signature mismatch!\n");
		safeQuit(1);
	}

	// read musH and fix endianness
	myfread(&musH->songLen,                  sizeof(short), 1, fileP);
	myfread(&musH->songOffset,               sizeof(short), 1, fileP);
	myfread(&musH->primaryChannels,          sizeof(short), 1, fileP);
	myfread(&musH->secondaryChannels,        sizeof(short), 1, fileP);
	myfread(&musH->instNum,                  sizeof(short), 1, fileP);
	myfread(&musH->reserved,                 sizeof(short), 1, fileP);

	musH->songLen            = SHORT ( musH->songLen                );
	musH->songOffset         = SHORT ( musH->songOffset             );
	musH->primaryChannels    = SHORT ( musH->primaryChannels        );
	musH->secondaryChannels  = SHORT ( musH->secondaryChannels      );
	musH->instNum            = SHORT ( musH->instNum                );
	musH->reserved           = SHORT ( musH->reserved               );

	// optionally print musH
	if (verbose)
		printf( "Metadata:                     \n"
			"  bytes        song length: %d\n"
			"  bytes        song offset: %d\n"
			"  no#     primary channels: %d\n"
			"  no#   secondary channels: %d\n"
			"  no#   instrument patches: %d\n"
			"        instrument patches: [ ",
			musH->songLen,
			musH->songOffset,
			musH->primaryChannels,
			musH->secondaryChannels,
			musH->instNum
                );

	
	musH->inst              = malloc( musH->instNum * sizeof(short) );
	for(int i = 0; i < musH->instNum; i++)
	{
		myfread( &musH->inst[i],       sizeof(short), 1, fileP        );
		musH->inst[i]        = SHORT ( musH->inst[i]                );
		if (verbose)
		{
			// In desperate need of beautification
			printf(i < (musH->instNum - 1) ? "%3d, " : "%3d", musH->inst[i]);
			if (i % FARRWRAP == (FARRWRAP-1) && i < (musH->instNum - 1))
				printf("\n%30s", ""); // URGGHHHHHHHH
		}
	}

	if ( verbose )
	{
		printf(" ]\n");

		// UNRELATED! Just also happens in verbose mode so I condensed the if statements
		long int ftold = ftell(fileP);
		printf("file cursor: %ld -> %d | %ld bytes unused\n", ftold, musH->songOffset, (long)musH->songOffset-ftold);

	}

	return;
}


musevents_t mus_readEvents(FILE *fileP, musheader_t musH) 
{
	if (verbose) printf("--- BEGIN READING EVENTS ---\n");

	if (songData != NULL) free(songData);
	// allocate memory for the sound data
	songData = (unsigned char*) malloc(musH.songLen);
	myfread( songData, 1, musH.songLen, fileP );

	// Process the song
	musevent_t    currentEvent = { 0 };
	unsigned char prevVol      =   0;
	musevents_t   eventsList   = {
		.count  = 0,
		.events = malloc(musH.songLen * sizeof(musevent_t)) // will always allocate enough, we can shorten it down later. Sorry memory optimizers!
	};

	for(short i = 0; i < musH.songLen; i++)
	{
		memset(&currentEvent, 0, sizeof(currentEvent));
		currentEvent.last =  (songData[i] & 0x80) != 0;
		currentEvent.type =  (songData[i] >>   4) &  0x07;
		currentEvent.chan =  (songData[i]       ) &  0x0f;
		if (verbose) printf("#%d ", eventsList.count);
		switch(currentEvent.type)
		{
			case RELEASE_NOTE:
				i++;
				currentEvent.data.rnote            = songData[i] & 0x7f; // mask with 0x7f as high flag is always zero here, so ignore bad highflags
				if (verbose) printf("Release note %d on channel %d\n", currentEvent.data.rnote, currentEvent.chan);
				break;

			case PLAY_NOTE:
				i++;
				currentEvent.data.pnote[0]         = songData[i] & 0x7f; // mask with 0x7f as high flag to ignore if vol
				
				currentEvent.data.pnote[2] = (songData[i] & 0x80) != 0;
				
				if (currentEvent.data.pnote[2] == 1)
				{
					i++;
					currentEvent.data.pnote[1]     = songData[i] & 0x7f; // mask with 0x7f as high flag is always zero here, so ignore bad highflags
					prevVol                        = currentEvent.data.pnote[1];
				} else
					currentEvent.data.pnote[1] = 0;

				if (verbose) printf("Play note %d [vol=%d,using %d] on channel %d\n", currentEvent.data.pnote[0], currentEvent.data.pnote[1], prevVol, currentEvent.chan);
				break;

			case PITCH_BEND:
				i++;
				currentEvent.data.pbend            = songData[i];
				if (verbose) printf("Pitch bend %f tones on channel %d\n", ((float)currentEvent.data.pbend/128.0f) - 1.0f, currentEvent.chan);
				break;

			case SYSTEM_EVENT:
				i++;
				currentEvent.data.sevnt            = songData[i] & 0x7f;
				if (currentEvent.data.sevnt < 10 || currentEvent.data.sevnt > 14)
				{
					fprintf(stderr, "Bad system event controller! %d", currentEvent.data.sevnt);
					safeQuit(1);
				}
				if (verbose) printf("System event %d/MIDI %d on channel %d\n", currentEvent.data.sevnt, cntrlToMidi[currentEvent.data.sevnt], currentEvent.chan);
				break;

			case CONTROLLER:
				i++;
				currentEvent.data.cntrl[0]         = songData[i] & 0x7f;
				if (currentEvent.data.cntrl[0] > 9)
				{
					fprintf(stderr, "Bad controller! %d", currentEvent.data.cntrl[0]);
					safeQuit(1);
				}
				i++;
				currentEvent.data.cntrl[1]         = songData[i] & 0x7f;
				if (verbose) printf("Controller %d/MIDI %d [val=%d] on channel %d\n", currentEvent.data.cntrl[0], cntrlToMidi[currentEvent.data.cntrl[0]], currentEvent.data.cntrl[1], currentEvent.chan);
				break;

			case MEASURE_END:
				currentEvent.data.mrend            = 0xff;
				if (verbose) printf("Measure end on channel %d\n", currentEvent.chan);
				break;

			case FINISH:
				currentEvent.data.finsh            = 0xaa;
				if (verbose) printf("Finished on channel %d\n", currentEvent.chan);
				break;

			case UNUSED:
				i++;
				currentEvent.data.finsh            = songData[i];
				if (verbose) printf("Unused byte %d on channel %d\n", songData[i], currentEvent.chan);
				break;

			// there is intentionally no default switch, read: https://aserebryakov.github.io/c++/2024/11/01/no-default-in-switch.html
		}

		if (currentEvent.last)
		{
			for(;;) // thank choccy doom for this
			{
				i++;
				currentEvent.wait *= 128; 
				currentEvent.wait += songData[i] & 0x7f;
				if((songData[i] & 0x80) == 0) break; // check the high bit (was originally ((songData >> 7) & 0x01) == 0 but that sucked - thank chocolate doom for this)
			}
			if (verbose) printf("-- [Delay %ld] --\n", currentEvent.wait);
		}
		memcpy(&eventsList.events[eventsList.count], &currentEvent, sizeof(musevent_t));
		eventsList.count++;
	}
	if (verbose) printf("---  END READING EVENTS  ---\n");

	// reallocate the array
	musevent_t *tmp = realloc(eventsList.events, eventsList.count * sizeof(musevent_t));
	if (tmp == NULL)
	{
		if(eventsList.events != NULL) free(eventsList.events);
		fprintf(stderr, "Realloc failed!\n");
		safeQuit(1);
	}
	eventsList.events = tmp;
	
	return eventsList;
}

void mus_shutdown (void)
{
	if ( songData    != NULL ) free(songData);
}