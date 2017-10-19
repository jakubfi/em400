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
#include <arpa/inet.h>
#include <sys/time.h>
#include <time.h>
#include <termios.h>
#include <fcntl.h>

// -----------------------------------------------------------------------
// convert an integer to formatted string with its binary representation
char * int2binf(char *format, uint64_t value, int size)
{
	char *i = format;
	char *buf = malloc(strlen(format)+1);
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
speed_t serial_int2speed(int s)
{
	struct a_speeds {
		int speed_i;
		speed_t speed_s;
	};
	struct a_speeds speeds[] = {
		{ 50,		B50 },
		{ 75,		B75 },
		{ 110,		B110 },
		{ 134,		B134 },
		{ 150,		B150 },
		{ 200,		B200 },
		{ 300,		B300 },
		{ 600,		B600 },
		{ 1200,		B1200 },
		{ 1800,		B1800 },
		{ 2400,		B2400 },
		{ 4800,		B4800 },
		{ 9600,		B9600 },
		{ 19200,	B19200 },
		{ 38400,	B38400 },
		{ 57600,	B57600 },
		{ 115200,	B115200 },
		{ 230400,	B230400 },
		{ 460800,	B460800 },
		{ 500000,	B500000 },
		{ 576000,	B576000 },
		{ 921600,	B921600 },
		{ 1000000,	B1000000 },
		{ 1152000,	B1152000 },
		{ 1500000,	B1500000 },
		{ 2000000,	B2000000 },
		{ 2500000,	B2500000 },
		{ 3000000,	B3000000 },
		{ 3500000,	B3500000 },
		{ 4000000,	B4000000 },
		{ 0,		-1 }
	};
	struct a_speeds *sp = speeds;

	while (sp->speed_i) {
		if (sp->speed_i == s) break;
		sp++;
	}

	return sp->speed_s;
}

// -----------------------------------------------------------------------
int serial_open(char *device, speed_t speed)
{
	int fd = open(device, O_RDWR | O_NOCTTY | O_SYNC);
	if (fd > 0) {
		struct termios tty;
		memset(&tty, 0, sizeof tty);
		tcgetattr(fd, &tty);
		cfsetospeed(&tty, speed);
		cfsetispeed(&tty, speed);
		tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
		tty.c_iflag &= ~IGNBRK;
		tty.c_iflag &= ~INLCR;
		tty.c_iflag &= ~ICRNL;
		tty.c_lflag = 0;
		tty.c_oflag = 0;
		tty.c_cc[VMIN]  = 1; // 1=blocking, 0=non-blocking
		tty.c_cc[VTIME] = 5;
		tty.c_iflag &= ~(IXON | IXOFF | IXANY);
		tty.c_cflag |= (CLOCAL | CREAD);
		tty.c_cflag &= ~(PARENB | PARODD);
		tty.c_cflag &= ~CSTOPB;
		tcsetattr(fd, TCSANOW, &tty);
	}

	return fd;
}

// vim: tabstop=4 shiftwidth=4 autoindent
