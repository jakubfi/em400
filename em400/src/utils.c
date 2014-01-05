//  Copyright (c) 2012-2013 Jakub Filipowicz <jakubf@gmail.com>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc.,
//  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <ctype.h>

// -----------------------------------------------------------------------
// convert an integer to formatted string with its binary representation
char * int2binf(char *format, uint64_t value, int size)
{
    char *i = format;
    char *buf = malloc(strlen(format)+1);
	char *o = buf;

    size--;

    while (*i) {
        switch (*i) {
    	    case '.':
	            if (size >= 0) {
                	*o = (value >> size) & 1 ? '1' : '0';
            	    size--;
        	    } else {
    	            *o = '?';
	            }
            	break;
        	default:
    	        *o = *i;
	            break;
        }
		o++;
        i++;
    }
	*o = '\0';
	return buf;
}

// -----------------------------------------------------------------------
char * int2chars(uint16_t w, char *buf)
{
	buf[0] = (w & 0b1111111100000000) >> 8;
	if (!isprint((unsigned)buf[0])) buf[0] = '.';
	buf[1] = (w & 0b0000000011111111);
	if (!isprint((unsigned)buf[1])) buf[1] = '.';
	buf[2] = '\0';
	return buf;
}

// -----------------------------------------------------------------------
char r2a(int i)
{
	if ((i >= 1) && (i <= 26)) {
		return i+64;
	} else if ((i >= 27) && (i <= 36)) {
		return i+21;
	} else if (i == 37) {
		return '_';
	} else if (i == 38) {
		return '%';
	} else if (i == 39) {
		return '#';
	} else {
		return '.';
	}
}

// -----------------------------------------------------------------------
char *int2r40(uint16_t w)
{
	char *buf = malloc(4);
	buf[0] = r2a((w/1600)%40);
	buf[1] = r2a((w/40)%40);
	buf[2] = r2a(w%40);
	buf[3] = '\0';
	return buf;
}

// vim: tabstop=4 shiftwidth=4 autoindent
