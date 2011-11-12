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

