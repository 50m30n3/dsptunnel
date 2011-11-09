#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <poll.h>
#include <limits.h>

#include "dsptunnel.h"

#include "output.h"

#define DATABUFFERSIZE 2048

static unsigned char databuffer[DATABUFFERSIZE];

#define AUDIOBUFFERSIZE 1024*2

static short int audiobuffer[AUDIOBUFFERSIZE];
static int bufferpos;

static int audio_out( int dsp, short int left, short int right )
{
	if( bufferpos >= AUDIOBUFFERSIZE )
	{
		if( write( dsp, audiobuffer, sizeof( short int ) * AUDIOBUFFERSIZE ) != sizeof( short int ) * AUDIOBUFFERSIZE )
		{
			perror( "audio_out: write" );
			return 0;
		}
		bufferpos = 0;
	}

	audiobuffer[bufferpos++] = left;
	audiobuffer[bufferpos++] = right;

	return 1;
}

void *output_loop( void *inopts )
{
	struct threadopts opts = *(struct threadopts *)inopts;
	
	struct pollfd pollfd;

	int i, j;

	unsigned char data;
	int size;

	short int lsample, rsample;
	int state, lastflip, laststate;

	bufferpos = 0;

	pollfd.fd = opts.tundev;
	pollfd.events = POLLIN;

	state = -1;
	laststate = -1;
	lastflip = 0;

	while( ! *(opts.done) )
	{
		if( poll( &pollfd, 1, 0 ) )
		{
			size = read( opts.tundev, databuffer, DATABUFFERSIZE );
			if( size == -1 )
			{
				perror( "output_loop: read" );
				return NULL;
			}

			fprintf( stderr, "Packet out: %i bytes\n", size );

			for( i=0; i<size; i++ )
			{
				data = databuffer[i];

				for( j=0; j<8; j++ )
				{
					if( data & 0x01 )
					{
						if( laststate&0x01 )
						{
							if( laststate&0x02 )
								state = 0x01;
							else
								state = 0x03;
						}
						else
						{
							if( lastflip )
								state = 0x01;
							else
								state = 0x03;

							lastflip = laststate&0x02;
						}
					}
					else
					{
						if( laststate&0x01 )
						{
							if( lastflip )
								state = 0x00;
							else
								state = 0x02;

							lastflip = laststate&0x02;
						}
						else
						{
							if( laststate&0x02 )
								state = 0x00;
							else
								state = 0x02;
						}
					}

					laststate = state;

					data >>= 1;
				
					if( state == 0 )
					{
						lsample = SHRT_MIN;
						rsample = SHRT_MIN;
					}
					else if( state == 1 )
					{
						lsample = SHRT_MIN;
						rsample = SHRT_MAX;
					}
					else if( state == 2 )
					{
						lsample = SHRT_MAX;
						rsample = SHRT_MAX;
					}
					else if( state == 3 )
					{
						lsample = SHRT_MAX;
						rsample = SHRT_MIN;
					}
				
					if( ! audio_out( opts.dspdev, lsample, rsample ) )
						return NULL;

					if( ! audio_out( opts.dspdev, lsample, rsample ) )
						return NULL;
				}
			}

			for( j=0; j<32; j++ )
			{
				if( ! audio_out( opts.dspdev, 0, 0 ) )
					return NULL;
			}
		}
		else
		{
			if( ! audio_out( opts.dspdev, 0, 0 ) )
				return NULL;
		}
	}

	return NULL;
}

