#ifndef DSPTUNNEL_H
#define DSPTUNNEL_H

struct threadopts
{
	int tundev;
	int dspdev;
	volatile int *done;
};

#endif

