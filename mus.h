/*
 * mustool - an all-in-one DMX audio library conversion tool
 * Copyleft Max Parry 2025 
 *
 * This is free software released into the public domain.
 *
 * mus.h - MUS file related headers
 */


#ifndef MUS_H
#define MUS_H

#include <stdbool.h>
#include <stdio.h>

// ref: https://archive.li/fJbpl (original https://moddingwiki.shikadi.net/wiki/MUS_Format#File_Format)
typedef struct {
	unsigned char  sig[4];            // MUS, 0x1A
	short          songLen;           // Length of the song in bytes
	short          songOffset;        // Offset of the song in bytes, relative to file start
	short          primaryChannels;   // Number of primary channels used in this song   - starts chan 0 
	short          secondaryChannels; // Number of secondary channels used in this song - starts chan 10 
	short          instNum;           // Number of instrument patches
	short          reserved;          // ???
	short         *inst;              // Pointer to array of short instruments
} musheader_t;

typedef union {
	unsigned char rnote;
	unsigned char pnote[3];
	unsigned char pbend;
	unsigned char sevnt;
	unsigned char cntrl[2];
	unsigned char mrend;
	unsigned char finsh;
	unsigned char uused;
} museventdata_t;

typedef enum {
	RELEASE_NOTE = 0, // 1   data byte  - the keyup   of MUS; Releases the note number, high flag is always zero
	PLAY_NOTE    = 1, // 1-2 data bytes - the keydown of MUS; Plays the given note number, where high flag represents whether there is a subsequent volume byte
	PITCH_BEND   = 2, // 1   data byte  - Bend all notes by the given ammount, where 0 is one tone down, 255 is one tone up, 128 is no bend. High flag included with no special meaning.
	SYSTEM_EVENT = 3, // 1   data byte  - Controls shit (see ref) - high flag is always zero
	CONTROLLER   = 4, // 2   data bytes - Controls shit (see ref) - high flag is always zero
	MEASURE_END  = 5, // 0   data bytes - Flags the end of the current musical measure
	FINISH       = 6, // 0   data bytes - Signals the end of data - typically implies a loop
	UNUSED       = 7  // 1   data byte  - Can be literally anything
} musevent_type;

typedef struct {
	bool           last; // 1-bit     | Delay? - process next event, then all future bytes do d (delay) = (d x 128) + (curByte & 0x7f) until the high bit isn't set
	musevent_type  type; // 3-bit     | Type of event
	short          chan; // 4-bit     | Channel
	museventdata_t data; //           | Data union
	long           wait; // 64/32-bit | Stores the delay
} musevent_t;

typedef struct {
	short         count; // Yes, this is what it sounds like
	musevent_t   *events; // Pointer to the events
} musevents_t;
static const unsigned char mus_sig[4] = {
	'M', 'U', 'S', '\x1a'
};

static const unsigned char cntrlToMidi[15] = {
	//// CONTROLLER ////
	0x00, //  0 change instrument, 0x00 because it's handled differently
	0x20, //  1 bank select   (fine), can be coarse according to the spec - bank 0 by default
	0x01, //  2 modulation   (coarse), frequency vibrato depth 
	0x07, //  3 volume       (coarse), 0-silent, ~100-normal, 127-loud 
	0x0A, //  4 pan/balance  (coarse), 0-left, 64-center (default), 127-right 
	0x0B, //  5 expression   (coarse), percentage of volume, divides the current volume into 128 steps 
	0x5B, //  6 reverb depth  / MIDI:  effects level
	0x5D, //  7 chorus depth  / MIDI:  chorus level
	0x40, //  8 sustain pedal / MIDI:  hold pedal
	0x43, //  9 soft pedal    / MIDI:  soft pedal
    //// SYSTEM EVT ////
	0x78, // 10 all sounds off
	0x7B, // 11 all notes off
	0x7E, // 12 mono
	0x7F, // 13 poly
	0x79  // 14 reset all controllers
};

void        mus_shutdown  (void);
void        mus_readHeader(FILE*, musheader_t*);
musevents_t mus_readEvents(FILE*, musheader_t );

#ifdef RAPTOR
#define MUS_PLAYBACKRATE 70
#else
#define MUS_PLAYBACKRATE 140
#endif

#endif