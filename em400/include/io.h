//  Copyright (c) 2012 Jakub Filipowicz <jakubf@gmail.com>
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

#ifndef IO_H
#define IO_H

#include <pthread.h>

#define IO_MAX_CHAN	16
#define IO_MAX_UNIT	8

enum io_dir {
	IO_IN,
	IO_OU
};

enum io_result {
	IO_NO = 0,  // no channel, no control unit, or no memory block
	IO_EN = 1,  // not ready
	IO_OK = 2,  // OK
	IO_PE = 3   // data error (parity error?)
};

enum io_chan_type {
	CHAN_NONE = 0,
	CHAN_CHAR,
	CHAN_MEM,
	CHAN_PI,
	CHAN_MULTIX,
	CHAN_PLIX
};

struct chan_t {
	int type;
	pthread_t thread;
};

extern struct chan_t io_channels[];

int io_init();
void io_shutdown();
int io_dispatch(int dir, uint16_t n, unsigned short int r);

#endif

// vim: tabstop=4
