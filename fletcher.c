
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

