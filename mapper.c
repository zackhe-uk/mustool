/*
 * mustool - an all-in-one DMX audio library conversion tool
 * Copyleft Max Parry 2025
 *
 * This is free software released into the public domain.
 *
 * mapper.c - Channel mapper
 */

#include "mus.h"

// ref: https://moddingwiki.shikadi.net/wiki/MUS_Format#Channel_mapping
short map_musToMid(short chan, musheader_t musH)
{
    short pChan = chan;
    
    if(chan == 15) 
    {
        // MUS chan percussion mapping
        pChan = 9;
    }
    return pChan;
}