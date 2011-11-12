/*
*    This file is part of dsptunnel.
*
*    dsptunnel is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    (at your option) any later version.
*
*    dsptunnel is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with dsptunnel.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <poll.h>
#include <limits.h>

#include "dsptunnel.h"
#include "fletcher.h"

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

	int i, j, k;

	unsigned char data;
	unsigned short int checksum;
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
			size = read( opts.tundev, databuffer, DATABUFFERSIZE-2 );
			if( size == -1 )
			{
				perror( "output_loop: read" );
				return NULL;
			}

			checksum = fletcher16( databuffer, size );
			
			databuffer[size++] = (checksum>>8)&0xff;
			databuffer[size++] = checksum&0xff;

			fprintf( stderr, "< %i bytes, checksum: 0x%04hX\n", size, checksum );

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

					for( k=0; k<opts.bitlength; k++ )
					{
						if( ! audio_out( opts.dspdev, lsample, rsample ) )
							return NULL;
					}
				}
			}

			for( i=0; i<16*opts.bitlength; i++ )
			{
				if( ! audio_out( opts.dspdev, 0, 0 ) )
					return NULL;
			}
		}
		else
		{
			for( i=0; i<16*opts.bitlength; i++ )
			{
				if( ! audio_out( opts.dspdev, 0, 0 ) )
					return NULL;
			}
		}
	}

	return NULL;
}

