//  Copyright (c) 2012-2025 Jakub Filipowicz <jakubf@gmail.com>
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

#ifndef UZDAT_H
#define UZDAT_H

#include "io/cchar/cchar.h"

typedef struct uzdat_s uzdat_t;

struct uzdat_s {
	cchar_unit_t base;

	pthread_mutex_t mutex;
	int intspec;
	int state;
	int dir;
	bool xfer_busy;
	char buf_wr;
	int buf_rd;

	em400_dev_t *dev;

	uv_async_t async_write;
	uv_async_t async_switch_transmit;
	uv_timer_t timer_switch_transmit;
};

cchar_unit_t * uzdat_create(int dev_num, em400_dev_t *dev);
void uzdat_on_data_received(uzdat_t *uzdat, char data);
void uzdat_on_data_sent(uzdat_t *uzdat);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
