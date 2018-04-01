//  Copyright (c) 2018 Jakub Filipowicz <jakubf@gmail.com>
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

#ifndef FDBRIDGE_H
#define FDBRIDGE_H

#include <inttypes.h>

#include "elst.h"

#ifdef __cplusplus
extern "C" {
#endif

enum fdbridge_event_types {
	EV_DATA = 10000,
	EV_EOF,
	EV_CONNECT,
	EV_ERROR,
};

struct fdbridge_event {
	int type;
	int sender;
	int len;
	void *ptr;
};

int fdbridge_init(unsigned fdcount);
void fdbridge_destroy();
void fdbridge_ev_free(struct fdbridge_event *e);

int fdbridge_add_stdin(ELST q);
int fdbridge_add_tcp(uint16_t port, ELST q);

#ifdef __cplusplus
extern "C" {
#endif

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
