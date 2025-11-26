/*
 * mustool - an all-in-one DMX audio library conversion tool
 * Copyleft Max Parry 2025 
 *
 * This is free software released into the public domain.
 *
 * shared.c - Shared functions
 */

#include <stdio.h>
#include <stdlib.h>

#include "main.h"
#include "mus.h"
#include "shared.h"

unsigned short SwapSHORT(unsigned short x)
{
    // No masking with 0xFF should be necessary. 
    return (x >> 8) | (x << 8);
}

// Swapping 32bit.
unsigned int SwapLONG(unsigned int x)
{
    return
	   (x >> 24)
	| ((x >>  8) & 0xff00)
	| ((x <<  8) & 0xff0000)
	|  (x << 24);
}

// Convenience.
size_t myfread  ( void *buffer, size_t size, size_t count,
            FILE *stream) 
{
	const size_t frRet = fread(buffer, size, count, stream);
	if ( frRet != count )
	{
		if (feof(stream))
			fprintf(stderr, "Unexpected EOF!\n");
		else if (ferror(stream))
			fprintf(stderr, "Error reading file\n");
		else
			fprintf(stderr, "Miscellaneous file reading error\n");
		fprintf(stderr, "returned size: %zu | expected %zu * %zu\n", frRet * size, size, count);
		safeQuit(1);
	}
	return frRet;
}

size_t myfwrite ( void *buffer, size_t size, size_t count,
            FILE *stream) 
{
	const size_t fwRet = fwrite(buffer, size, count, stream);
	if ( fwRet != count )
	{
		if (feof(stream))
			fprintf(stderr, "Unexpected EOF!\n");
		else if (ferror(stream))
			fprintf(stderr, "Error reading file\n");
		else
			fprintf(stderr, "Miscellaneous file writing error\n");
		fprintf(stderr, "returned size: %zu | expected %zu * %zu\n", fwRet * size, size, count);
		safeQuit(1);
	}
	return fwRet;
}

void safeQuit (int retCode)
{
    main_shutdown();
    mus_shutdown();
	
	exit(retCode);
}