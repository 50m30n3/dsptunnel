#ifndef DSPTUNNEL_H
#define DSPTUNNEL_H

struct threadopts
{
	int tundev;
	int dspdev;
	int bitlength;
	volatile int *done;
};

#endif

