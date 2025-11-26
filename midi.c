/*
 * mustool - an all-in-one DMX audio library conversion tool
 * Copyleft Max Parry 2025
 *
 * This is free software released into the public domain.
 *
 * midi.c - MIDI writer
 */


#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "mus.h"
#include "shared.h"
#include "mapper.h"

#include "midi.h"

unsigned long delayQueue = 0;

void midi_writeHeader(FILE *fileP)
{
    midiheader_t toSetH = {
        .sig        = 0,                  // weird C quirk
        .headerLen  = 6,
        .type       = 0,                  // type-0 MIDI file,
        .trackCount = 1,                  // with one track.
        .ticksPerQN = MUS_PLAYBACKRATE/2
    };
    memcpy(&toSetH.sig, &midi_sig, sizeof(midi_sig));

    toSetH.headerLen  =  LONGBE(toSetH.headerLen );
    toSetH.type       = SHORTBE(toSetH.type      );
    toSetH.trackCount = SHORTBE(toSetH.trackCount);
    toSetH.ticksPerQN = SHORTBE(toSetH.ticksPerQN);
    
    myfwrite(&toSetH.sig,        sizeof(unsigned char), sizeof(midi_sig), fileP);
    myfwrite(&toSetH.headerLen,  sizeof(int),                          1, fileP);
    myfwrite(&toSetH.type,       sizeof(short),                        1, fileP);
    myfwrite(&toSetH.trackCount, sizeof(short),                        1, fileP);
    myfwrite(&toSetH.ticksPerQN, sizeof(short),                        1, fileP);

}

#ifdef DELAY_ALT
// from mus2midi, though my version *should* be functionally the same
void midi_writeTime2(unsigned long time, FILE *fileP)
{
    unsigned long buffer = time & 0x7F;
    unsigned char writeVal;

    while ((time >>= 7) != 0)
    {
        buffer <<= 8;
        buffer |= ((time & 0x7F) | 0x80);
    }

    for (;;)
    {
        writeVal = (unsigned char)(buffer & 0xFF);

        myfwrite(&writeVal, 1, 1, fileP);

        if ((buffer & 0x80) != 0) buffer >>= 8;
        else 
        {
            delayQueue = 0;
            return;
        }
    }
}
#endif

void midi_writeTime(FILE *fileP, musevent_t currentEvent)
{
#ifdef DELAY_ALT
    midi_writeTime2(delayQueue, fileP);
#else
    unsigned long delayVal = delayQueue;
    unsigned char delayC = 0;
    if(delayQueue != 0) 
    {
        for(int j = 0; j < sizeof(long) && delayVal != 0; j++)
        {
            if (verbose) printf("Delay: [%d, %zu, %ld]\n", j, delayVal, delayQueue);
            delayC = (delayVal & 0x7f) | ((delayVal >> 7) != 0 ? 0x80 : 0);
            myfwrite(&delayC, sizeof(unsigned char), 1, fileP);
            delayVal >>= 7;
        }
        delayQueue = 0;
    }
    else
    {
        int tmp = 0;
        myfwrite(&tmp, sizeof(unsigned char), 1, fileP);
    }
    
#endif
}
void midi_writeFromMus(FILE *fileP, musevents_t eventsList, musheader_t musH)
{
    int tmp = 0;
    myfwrite("MTrk", sizeof(char),                        4, fileP);
    int lengthSeek = ftell(fileP);
    myfwrite(&tmp,   sizeof(int),                         1, fileP);

	musevent_t    currentEvent = { 0 };

    for(short i = 0; i < eventsList.count; i++)
    {
        currentEvent = eventsList.events[i];
        unsigned char midiChan = (unsigned char)map_musToMid(currentEvent.chan);
        //if (verbose) printf("CHAN intended: %d, actual: %d\n", currentEvent.chan, midiChan);

        // now, for the actual event handling
        switch(currentEvent.type)
		{
			case RELEASE_NOTE:
                // note-off
                midi_writeTime(fileP, currentEvent);
                {
                    unsigned char tmpBuf[3] = {
                        0x80 | midiChan,
                        currentEvent.data.rnote,
                        0 // should be 127... but choccy doom uses 0? I'm just going off http://midi.teragonaudio.com/tech/midispec/noteoff.htm here
                    };
                    myfwrite(&tmpBuf, sizeof(unsigned char), sizeof(tmpBuf), fileP);
                }
				break;

			case PLAY_NOTE:
                // note-off
                midi_writeTime(fileP, currentEvent);
                {
                    unsigned char tmpBuf[3] = {
                        0x90 | midiChan,
                        currentEvent.data.pnote[0],
                        (currentEvent.data.pnote[2] == 1) ? currentEvent.data.pnote[1] : 64 // 64 is the middle of the volume, it will serve well here
                    };
                    myfwrite(&tmpBuf, sizeof(unsigned char), sizeof(tmpBuf), fileP);
                }
				break;

			case PITCH_BEND:
                // pitch bend/wheel
                midi_writeTime(fileP, currentEvent);
                {
                    short pbend = ((short)currentEvent.data.pbend) << 6; // Same as Chocolate Doom's times 64, only faster. Open a PR?
                    unsigned char tmpBuf[3] = {
                        0xe0 | midiChan,
                        pbend & 0x7f,
                        (pbend >> 7) & 0x7f
                    };
                    myfwrite(&tmpBuf, sizeof(unsigned char), sizeof(tmpBuf), fileP);
                }
				break;

			case SYSTEM_EVENT:
                // system event
                midi_writeTime(fileP, currentEvent);
                {
                    unsigned char tmpBuf[3] = {
                        0xb0 | midiChan,
                        cntrlToMidi[currentEvent.data.sevnt] & 0x7f,
                        0
                    };
                    myfwrite(&tmpBuf, sizeof(unsigned char), sizeof(tmpBuf), fileP);
                }
				break;

			case CONTROLLER:
                // controller
                midi_writeTime(fileP, currentEvent);
                if (currentEvent.data.cntrl[0] != 0)
                {
                    unsigned char tmpBuf[3] = {
                        0xb0 | midiChan,
                        cntrlToMidi[currentEvent.data.cntrl[0]] & 0x7f,
                        currentEvent.data.cntrl[1] & 0x7f
                    };
                    myfwrite(&tmpBuf, sizeof(unsigned char), sizeof(tmpBuf), fileP);
                }
                else
                {
                    unsigned char tmpBuf[2] = {
                        0xc0 | midiChan,
                        currentEvent.data.cntrl[1] & 0x7f
                    };
                    myfwrite(&tmpBuf, sizeof(unsigned char), sizeof(tmpBuf), fileP);
                }
				break;

			case MEASURE_END:
				break;

			case FINISH:
                // finish - loop back to beginning
                /*{
                    unsigned char tmpBuf[1] = {
                        0xfa
                    };
                    myfwrite(&tmpBuf, sizeof(unsigned char), sizeof(tmpBuf), fileP);
                }*/
				break;

			case UNUSED:
				break;

			// there is intentionally no default switch, read: https://aserebryakov.github.io/c++/2024/11/01/no-default-in-switch.html
		}
        if (currentEvent.type != FINISH) delayQueue += currentEvent.wait;
    }
    unsigned char endMidi[4] = { // the first 0x00 is the delay
        0x00, 0xFF, 0x2F, 0x00
    };
    myfwrite(&endMidi, sizeof(unsigned char), sizeof(endMidi), fileP);
    
    unsigned int trackLength = ftell(fileP) - (lengthSeek + sizeof(int));
    
    if ( fseek(fileP, lengthSeek, SEEK_SET) != 0 )
	{
		fprintf(stderr, "Could not seek file\n");
		safeQuit(1);
	}

    trackLength = LONGBE(trackLength);
    myfwrite(&trackLength, sizeof(int),                         1, fileP);
}