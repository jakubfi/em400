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

#ifndef CCHAR_H
#define CCHAR_H

#include <inttypes.h>
#include <stdbool.h>

#include "io/chan.h"

enum cchar_controller_types {
	CCHAR_UZDAT,
	CCHAR_FLOP8,
	CCHAR_CONTROLLER_COUNT
};

typedef struct chan_char chan_char_t;
typedef struct cchar_unit cchar_unit_t;

typedef void (*cchar_unit_f_shutdown)(cchar_unit_t *unit);
typedef void (*cchar_unit_f_free)(cchar_unit_t *unit);
typedef void (*cchar_unit_f_reset)(cchar_unit_t *unit);
typedef int (*cchar_unit_f_cmd)(cchar_unit_t *unit, int dir, int cmd, uint16_t *r_arg);
typedef int (*cchar_unit_f_intspec)(cchar_unit_t *unit);
typedef bool (*cchar_unit_f_has_interrupt)(cchar_unit_t *unit);

struct cchar_unit {
	int type;				// controller type
	int num;				// controller number in char channel
	chan_char_t *chan;		// channel

	cchar_unit_f_shutdown shutdown;
	cchar_unit_f_reset reset;
	cchar_unit_f_cmd cmd;
	cchar_unit_f_intspec intspec;
	cchar_unit_f_has_interrupt has_interrupt;
};

chan_t *cchar_create(int num);
void cchar_int_trigger(chan_char_t *chan);
void cchar_int_cancel(chan_char_t *chan, int unit_n);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
