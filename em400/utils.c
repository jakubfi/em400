#include <stdlib.h>

// -----------------------------------------------------------------------
// convert an integer to string with its binary representation
char* int2bin(int x, int len)
{
	char* buf = malloc(len+1);

	buf[len] = 0;

	for (int i=len-1 ; i>=0 ; i--) {
		buf[i] = 48 + ((x >> (len-(i+1))) & 1);
	}
	return buf;
}

