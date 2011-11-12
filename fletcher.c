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

unsigned short int fletcher16( unsigned char * data, int length )
{
	unsigned char s1, s2;
	int i;
	
	s1 = s2 = 0;
	
	for( i=0; i<length; i++ )
	{
		s1 += data[i];
		s2 += s1;
	}
	
	return (s2<<8)|s1;
}

