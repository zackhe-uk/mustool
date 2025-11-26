/*
 * mustool - an all-in-one DMX audio library conversion tool
 * Copyleft Max Parry 2025 
 *
 * This is free software released into the public domain.
 *
 * shared.h - Shared definitions
 */

#ifndef SHARED_H
#define SHARED_H

unsigned short SwapSHORT(unsigned short x);
unsigned int SwapLONG(unsigned int x);


#ifdef __BIG_ENDIAN__
#define SHORT(x) (SwapSHORT(x))
#define SHORTBE(x) (x)
#define LONG(x) (SwapLONG(x))
#define LONGBE(x) (x)
#else
#define SHORT(x) (x)
#define SHORTBE(x) (SwapSHORT(x))
#define LONG(x) (x)
#define LONGBE(x) (SwapLONG(x))
#endif


// program settings
extern int            verbose;

size_t myfread  (void *, size_t, size_t, FILE *);
size_t myfwrite (void *, size_t, size_t, FILE *);

void  safeQuit (int);

// formatting array wrap
#define FARRWRAP 5

#endif