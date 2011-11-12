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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>

#include "dsptunnel.h"

#include "tun.h"
#include "dsp.h"

#include "input.h"
#include "output.h"

volatile int done;

void sig_exit( int sig )
{
	fputs( "Quitting\n", stderr );
	done = 1;
}

int main( int argc, char *argv[] )
{
	int opt;

	int tundev;
	int dspdev;

	pthread_t inthread, outthread;
	struct threadopts opts;

	char *tunname = NULL;
	char *dspname = NULL;
	int samplerate = 48000;
	int bitlength = 2;

	opterr = 0;

	while( ( opt = getopt( argc, argv, "ht:d:s:b:" ) ) != -1 )
	{
		switch( opt )
		{
			case 'h':
				puts( "dsptunnel v.1.0 by 50m30n3 2011" );
				puts( "" );
				puts( "USAGE: dsptunnel [-h] [-t tunif] [-d dspdev] [-s samplerate] [-b bitlength]" );
				puts( "" );
				puts( "\t-h\t\tShow help" );
				puts( "\t-t tunif\tSet name of tunnel interface (tun0)" );
				puts( "\t-d dspdev\tSet name of dsp device (/dev/dsp)" );
				puts( "\t-s sampelerate\tSet the sample rate (48000)" );
				puts( "\t-b bitlength\tSet the length of one bit, in samples (2)" );
				return EXIT_SUCCESS;
			break;

			case 't':
				tunname = strdup( optarg );
			break;
			
			case 'd':
				dspname = strdup( optarg );
			break;

			case 's':
				samplerate = atoi( optarg );
			break;

			case 'b':
				bitlength = atoi( optarg );
			break;

			default:
			case '?':
				fputs( "dsptunnel: main: Can not parse command line\n", stderr );
				return EXIT_FAILURE;
			break;
		}	
	}
	
	if( ! tunname )
		tunname = strdup( "tun0" );
	
	if( ! dspname )
		dspname = strdup( "/dev/dsp" );

	if( samplerate <= 0 )
	{
		fputs( "dsptunnel: main: Illegal sample rate\n", stderr );
		return EXIT_FAILURE;
	}

	if( bitlength <= 0 )
	{
		fputs( "dsptunnel: main: Illegal bit length\n", stderr );
		return EXIT_FAILURE;
	}

	tundev = tun_open( tunname );
	if( tundev < 0 )
		return EXIT_FAILURE;
	
	dspdev = dsp_open( dspname, samplerate );
	if( dspdev < 0 )
		return EXIT_FAILURE;

	signal( SIGINT, sig_exit );
	signal( SIGTERM, sig_exit );
	done = 0;

	opts.tundev = tundev;
	opts.dspdev = dspdev;
	opts.bitlength = bitlength;
	opts.done = &done;

	if( pthread_create( &inthread, NULL, input_loop, &opts ) != 0 )
	{
		perror( "dsptunnel: main: pthread_create: input_loop" );
		return EXIT_FAILURE;
	}

	if( pthread_create( &outthread, NULL, output_loop, &opts ) != 0 )
	{
		perror( "dsptunnel: main: pthread_create: output_loop" );
		return EXIT_FAILURE;
	}

	if( pthread_join( inthread, NULL ) != 0 )
	{
		perror( "dsptunnel: main: pthread_join: inthread" );
		return EXIT_FAILURE;
	}

	if( pthread_join( outthread, NULL ) != 0 )
	{
		perror( "dsptunnel: main: pthread_join: outthread" );
		return EXIT_FAILURE;
	}

	close( tundev );
	close( dspdev );

	free( tunname );
	free( dspname );
	
	return EXIT_SUCCESS;
}

