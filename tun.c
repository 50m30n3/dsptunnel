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

