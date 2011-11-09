#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_tun.h>

int tun_open( char *tunname )
{
	struct ifreq req;
	int dev;
	
	dev = open( "/dev/net/tun", O_RDWR );
	if( dev == -1 )
	{
		perror( "dsptunnel: tun_open: open" );
		return -1;
	}
	
	memset( &req, 0, sizeof( req ) );
	req.ifr_flags = IFF_TUN | IFF_NO_PI;
	strncpy( req.ifr_name, tunname, IFNAMSIZ );
	
	if( ioctl( dev, TUNSETIFF, &req ) < 0 )
	{
		perror( "dsptunnel: tun_open: ioctl" );
		close( dev );
		return -1;
	}
	
	return dev;
}

