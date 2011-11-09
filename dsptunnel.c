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

	opterr = 0;

	while( ( opt = getopt( argc, argv, "ht:d:" ) ) != -1 )
	{
		switch( opt )
		{
			case 'h':
				puts( "dsptunnel v.1.0 by 50m30n3 2011" );
				puts( "" );
				puts( "USAGE: dsptunnel [-h] [-t tunif] [-d dspdev]" );
				puts( "" );
				puts( "\t-h\t\tShow help" );
				puts( "\t-t tunif\tSet name of tunnel interface (tun0)" );
				puts( "\t-d dspdev\tSet name of dsp device (/dev/dsp)" );
				return EXIT_SUCCESS;
			break;

			case 't':
				tunname = strdup( optarg );
			break;
			
			case 'd':
				dspname = strdup( optarg );
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

	tundev = tun_open( tunname );
	if( tundev < 0 )
		return EXIT_FAILURE;
	
	dspdev = dsp_open( dspname );
	if( dspdev < 0 )
		return EXIT_FAILURE;

	signal( SIGINT, sig_exit );
	signal( SIGTERM, sig_exit );
	done = 0;

	opts.tundev = tundev;
	opts.dspdev = dspdev;
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

