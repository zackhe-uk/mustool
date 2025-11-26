/*
 * mustool - an all-in-one DMX audio library conversion tool
 * Copyleft Max Parry 2025
 *
 * This is free software released into the public domain.
 *
 * midi.c - MIDI writer headers
 *
 *
 * MIDI is the language of gods.
 *
 * Lesser life forms communicate in more primitive, less artistic
 * manners such as barks, chirps, roars, or any of the many human
 * verbal languages in use throughout the globe.
 *
 * But, gods communicate using MIDI messages. Even a picture,
 * whose value is often equated to a thousand words, cannot
 * match the scope of emotional and intellectual power unleashed
 * by a stream of MIDI messages. 
 *
 * But, try to tell that to the visual-and-print-fixated, tone-deaf heathen who infest this planet... 
 */

// ref: https://moddingwiki.shikadi.net/wiki/MID_Format
//      https://github.com/chocolate-doom/chocolate-doom/blob/master/src/mus2mid.c
//      https://drive.google.com/file/d/1t4jcCCKoi5HMi7YJ6skvZfKcefLhhOgU
//      http://midi.teragonaudio.com/tech/midispec.htm

#ifndef MIDI_H
#define MIDI_H

typedef struct {
    unsigned char sig[4];     // Signature          - MThd
    int           headerLen;  // Header length      - MUST be 6, in bytes
    short         type;       // Type               - MUST be 0
    short         trackCount; // Track count        - MUST be 1 for type 0
    short         ticksPerQN; // Ticks per 1/4 note - 70 for most, 35 for Raptor (orignal MUS game). Hz / 2
} midiheader_t;

static const unsigned char midi_sig[4] = {
    'M', 'T', 'h', 'd'
};

void midi_writeHeader(FILE *);
void midi_writeFromMus(FILE *, musevents_t, musheader_t);

#endif