/*
 * mustool - an all-in-one DMX audio library conversion tool
 * Copyleft Max Parry 2025 
 *
 * This is free software released into the public domain.
 *
 * main.c - Main program interface
 */



#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "mus.h"
#include "midi.h"

#include "shared.h"
#include "main.h"


// file data
FILE           *filePtr = NULL;
FILE           *midPtr  = NULL;
musheader_t      header = { 0 };

int             verbose = 0;

///////////////// HELPERS /////////////////

void main_shutdown (void)
{
	if ( header.inst != NULL ) free(header.inst);
	if ( filePtr     != NULL ) fclose(filePtr);
	if ( midPtr      != NULL ) fclose(midPtr);
}

void printHelp (char *progName)
{
	printf( "                            \n"
		"MUStool v1.0                \n"
		"                            \n"
		"Example: %s [file.mus]      \n"
		"  -help | print this message\n"
		"  -v    | be verbose        \n"
		"                            \n",
		progName
	);
}


/////////////// END HELPERS ///////////////


int main ( int argc, char **argv )
{
	if ( argc == 1 )
	{
		printHelp(argv[0]);
		return 0;
	}

	for ( int i = 1; i < argc; i++ )
	{
		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "-help") == 0 || strcmp(argv[i], "--help") == 0)
		{
			printHelp(argv[0]);
			safeQuit(0);
		}
		if (strcmp(argv[i], "-v") == 0)
			verbose = 1;


		if ( i == argc - 1 && argv[i][0] != '-' )
		{
			printf("Opening %s...\n", argv[i]);
			filePtr = fopen(argv[i], "rb");
		}
	}

	if ( filePtr == NULL ) {
		fprintf(stderr, "No valid file was passed\n");
		safeQuit(1);
	}

	mus_readHeader(filePtr, &header);

	if ( fseek(filePtr, header.songOffset, SEEK_SET) != 0 )
	{
		fprintf(stderr, "Could not seek file\n");
		safeQuit(1);
	}

	musevents_t eventsList = mus_readEvents(filePtr, header);

	midPtr = fopen("out.mid", "wb");

	midi_writeHeader(midPtr);
	midi_writeFromMus(midPtr, eventsList, header);


	safeQuit(0);
}

