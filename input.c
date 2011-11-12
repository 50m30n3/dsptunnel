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
#include <limits.h>

#include "dsptunnel.h"
#include "fletcher.h"

#include "input.h"

#define THRESHOLD SHRT_MAX / 4

#define DATABUFFERSIZE 2048
static unsigned char databuffer[DATABUFFERSIZE];

#define AUDIOBUFFERSIZE 1024*2
static short int audiobuffer[AUDIOBUFFERSIZE];
static int bufferpos;

static int audio_in( int dsp, short int *left, short int *right )
{
	if( bufferpos >= AUDIOBUFFERSIZE )
	{
		if( read( dsp, audiobuffer, sizeof( short int ) * AUDIOBUFFERSIZE ) != sizeof( short int ) * AUDIOBUFFERSIZE )
		{
			perror( "audio_in: read" );
			return 0;
		}
		bufferpos = 0;
	}

	*left = audiobuffer[bufferpos++];
	*right = audiobuffer[bufferpos++];
	
	return 1;
}

void *input_loop( void *inopts )
{
	struct threadopts opts = *(struct threadopts*)inopts;

	unsigned char data;
	unsigned short int checksum;
	int size, bits;
	
	short int lsample, rsample;
	int state, laststate, error, silence;
	
	bufferpos = AUDIOBUFFERSIZE;

	state = -1;
	laststate = -1;
	error = 0;
	silence = 0;

	size = 0;
	bits = 0;
	data = 0;

	while( ! *(opts.done) )
	{
		if( ! audio_in( opts.dspdev, &lsample, &rsample ) )
			return NULL;

		if( lsample < -THRESHOLD )
		{
			if( rsample < -THRESHOLD )
			{
				state = 0x00;
			}
			else if( rsample > THRESHOLD )
			{
				state = 0x01;
			}
			else
			{
				state = -1;
			}
		}
		else if( lsample > THRESHOLD )
		{
			if( rsample < -THRESHOLD )
			{
				state = 0x03;
			}
			else if( rsample > THRESHOLD )
			{
				state = 0x02;
			}
			else
			{
				state = -1;
			}
		}
		else
		{
			state = -1;
		}

		if( state != -1 )
		{
			silence = 0;

			if( ! error )
			{
				if( state != laststate )
				{
					data |= (state&0x01)<<bits;
					bits++;
				
					if( bits >= 8 )
					{
						databuffer[size++] = data;
					
						data = 0;
						bits = 0;
						
						if( size >= DATABUFFERSIZE )
						{
							error = 1;
							fputs( "input_loop: mtu exceeded\n", stderr );
						}
					}
				
					laststate = state;
				}
			}
		}
		else
		{
			silence++;
			if( silence >= 8*opts.bitlength )
			{
				if( ( size > 2 ) && ( ! error ) )
				{
					checksum = fletcher16( databuffer, size-2 );
					
					fprintf( stderr, "< %i bytes, checksum: 0x%04hX (0x%04hX)\n", size, ((databuffer[size-2]<<8) | databuffer[size-1]), checksum );
					
					if( ((databuffer[size-2]<<8) | databuffer[size-1]) == checksum )
					{
						if( write( opts.tundev, databuffer, size-2 ) != size-2 )
						{
							perror( "input_loop: write" );
						}
					}
					else
					{
						fputs( "input_loop: incorrect checksum\n", stderr );
					}
				}

				state = -1;
				laststate = -1;
				error = 0;

				size = 0;
				bits = 0;
				data = 0;
			}
		}
	}

	return NULL;
}

