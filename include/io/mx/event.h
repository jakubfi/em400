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

#ifndef MX_EVENT
#define MX_EVENT

#include <inttypes.h>

#include "utils/fdbridge.h"

// event types are also priorities for the event queue
enum mx_event_types {
	MX_EV_CMD,
	MX_EV_INT_PUSH,
	MX_EV_RESET,
	MX_EV_QUIT, // highest priority
	MX_EV_CNT
};

union mx_event {
	struct fdbridge_event fd;
	struct {
		int type;
		int cmd;
		int log_n;
		uint16_t arg;
		unsigned id;
	} d;
};

const char * mx_get_event_name(unsigned ev);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
