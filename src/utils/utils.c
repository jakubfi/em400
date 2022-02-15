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

#define _POSIX_C_SOURCE 199309L

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <ctype.h>
#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif
#include <sys/time.h>
#include <time.h>
#include <fcntl.h>

#include "utils/utils.h"

// -----------------------------------------------------------------------
// convert an integer to formatted string with its binary representation
char * int2binf(char *buf, const char *format, uint64_t value, int size)
{
	const char *i = format;
	char *o = buf;

	size--;

	while (*i) {
		if (*i == '.') {
			if (size >= 0) {
				*o = (value >> size) & 1 ? '1' : '0';
				size--;
			} else {
				*o = '?';
			}
		} else {
			*o = *i;
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
void endianswap(uint16_t *ptr, int size)
{
	while (size > 0) {
		size--;
		ptr[size] = ntohs(ptr[size]);
	}
}

// -----------------------------------------------------------------------
double stopwatch_ns()
{
	int mul = 1;

#ifdef CLOCK_REALTIME
	static struct timespec t, ot;
	clock_gettime(CLOCK_REALTIME, &t);
	double elapsed_us = 1000000000.0 * (t.tv_sec - ot.tv_sec) + (t.tv_nsec - ot.tv_nsec);
#else
	static struct timeval t, ot;
	gettimeofday(&t, NULL);
	double elapsed_us = 1000000.0 * (t.tv_sec - ot.tv_sec) + (t.tv_usec - ot.tv_usec);
	mul = 1000;
#endif

	ot = t;
	return elapsed_us * mul;
}

// -----------------------------------------------------------------------
int parity(unsigned int v)
{
	int parity = 0;
	while (v) {
		parity = !parity;
		v = v & (v-1);
	}
	return parity;
}

// -----------------------------------------------------------------------
void word2bin(uint16_t w, uint8_t *b)
{
	for (int i=2 ; i>=0 ; i--) {
		b[i] = (w & 0b00111111) | 0b01000000;
	    b[i] |= parity(b[i]) << 7;
		w >>= 6;
	}
}

// -----------------------------------------------------------------------
uint16_t bin2word(uint8_t *b)
{
	return ((b[0] & 0b1111) << 12) + ((b[1] & 0b111111) << 6) + (b[2] & 0b111111);
}

// -----------------------------------------------------------------------
int bin_is_end(uint8_t b)
{
	return ((b & BIN_ENDBYTE) == BIN_ENDBYTE);
}

// -----------------------------------------------------------------------
int bin_is_valid(uint8_t b)
{
	return (b & 0b01000000) != 0;
}

// vim: tabstop=4 shiftwidth=4 autoindent
