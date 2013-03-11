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

#include <inttypes.h>
#include <pthread.h>
#include <unistd.h>

#include "cfg.h"
#include "errors.h"
#include "io.h"
#include "drv/cchar.h"
#include "drv/lib.h"

// -----------------------------------------------------------------------
int drv_cchar_init(void *self, struct cfg_arg_t *arg)
{
	struct chan_t *ch = self;
	drv_cchar_reset(ch);
	return E_OK;
}

// -----------------------------------------------------------------------
void drv_cchar_shutdown(void *self)
{
	struct chan_t *ch = self;
	ch->finish = 1;
}

// -----------------------------------------------------------------------
void drv_cchar_reset(void *self)
{
	struct chan_t *ch = self;
	ch->int_spec = 0;
	ch->int_mask = 0;
}

// -----------------------------------------------------------------------
int drv_cchar_cmd(void *self, int u_num, int dir, int cmd, uint16_t *r)
{
	struct chan_t *ch = self;
	struct unit_t *unit = ch->unit[u_num];
	ch->int_mask = 0;

	// command for channel
	if ((cmd & 0b11100000) == 0) {
		if (dir == IO_OU) {
			switch (cmd & 0b00011000) {
			case CHAN_CMD_EXISTS:
				break;
			case CHAN_CMD_INTSPEC:
				chan_get_int_spec(ch, r);
				break;
			case CHAN_CMD_ALLOC:
				// all units always working with CPU 0
				*r = 0;
				break;
			default:
				// shouldn't happen, but as channel always reports OK...
				break;
			}
		} else {
			switch (cmd & 0b00011000) {
			case CHAN_CMD_EXISTS:
				break;
			case CHAN_CMD_MASK_PN:
				ch->int_mask = 1;
				break;
			case CHAN_CMD_MASK_NPN:
				// ignore 2nd CPU
				break;
			case CHAN_CMD_ASSIGN:
				// always for CPU 0
				break;
			default:
				// shouldn't happen, but as channel always reports OK...
				break;
			}
		}
		return IO_OK;
	// command for unit
	} else {
		return unit->f_cmd(unit, -1, dir, cmd, r);
	}
}

// vim: tabstop=4
