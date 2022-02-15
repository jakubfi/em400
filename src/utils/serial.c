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
#include <termios.h>
#include <fcntl.h>

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
		{ 0,		0 }
	};
	struct a_speeds *sp = speeds;

	while (sp->speed_i) {
		if (sp->speed_i == s) break;
		sp++;
	}

	return sp->speed_s;
}

// -----------------------------------------------------------------------
int serial_open(const char *device, speed_t speed)
{
	int fd = open(device, O_RDWR | O_NOCTTY | O_SYNC);
	if (fd > 0) {
		struct termios tty;
		memset(&tty, 0, sizeof tty);
		if (tcgetattr(fd, &tty) != 0) {
			return -1;
		}
		if (cfsetospeed(&tty, speed) != 0) {
			return -1;
		}
		if (cfsetispeed(&tty, speed) != 0) {
			return -1;
		}
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
		if (tcsetattr(fd, TCSANOW, &tty) != 0) {
			return -1;
		}
	}

	return fd;
}

// vim: tabstop=4 shiftwidth=4 autoindent
