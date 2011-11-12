#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/soundcard.h>

int dsp_open( char *dspname, int samplerate )
{
	int dev;
	int arg;

	dev = open( dspname, O_RDWR );
	if( dev < 0 )
	{
		perror( "dsp_open: fopen" );
		return -1;
	}

	arg = AFMT_S16_NE;
	if( ioctl( dev, SNDCTL_DSP_SETFMT, &arg ) == -1 )
	{
		perror( "dsp_open: ioctl: SNDCTL_DSP_SETFMT" );
		return -1;
	}

	if( arg != AFMT_S16_NE )
	{
		fputs( "dsp_open: Cannot set sample format\n", stderr );
		return -1;
	}

	arg = 2;
	if( ioctl( dev, SNDCTL_DSP_CHANNELS, &arg ) == -1 )
	{
		perror( "dsp_open: ioctl: SNDCTL_DSP_CHANNELS" );
		return -1;
	}

	if( arg != 2 )
	{
		fputs( "dsp_open: Cannot set number of channels\n", stderr );
		return -1;
	}

	arg = samplerate;
	if( ioctl( dev, SNDCTL_DSP_SPEED, &arg ) == -1 )
	{
		perror( "dsp_open: ioctl: SNDCTL_DSP_SPEED" );
		return -1;
	}
	
	if( arg != samplerate )
	{
		fputs( "dsp_open: Cannot set sample rate\n", stderr );
		return -1;
	}

	return dev;
}

