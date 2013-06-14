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

#include "io/io.h"
#include "io/chan.h"
#include "io/cchar.h"

#include "cfg.h"
#include "errors.h"

#include "debugger/log.h"

// -----------------------------------------------------------------------
struct chan_proto_t * cchar_create(struct cfg_unit_t *units)
{
	return E_OK;
}

// -----------------------------------------------------------------------
void cchar_shutdown(struct chan_proto_t *chan)
{
}

// -----------------------------------------------------------------------
void cchar_reset(struct chan_proto_t  *chan)
{
}

// -----------------------------------------------------------------------
int cchar_cmd(struct chan_proto_t *chan, int dir, uint16_t n_arg, uint16_t *r_arg)
{
	//int u_num = (n_arg & 0b0000000011100000) >> 5;
	int cmd = (n_arg & 0b1111111100000000) >> 8;

	//chan->int_mask = 0;

	// command for channel
	if ((cmd & 0b11100000) == 0) {
		if (dir == IO_OU) {
			switch (cmd & 0b00011000) {
			case CHAN_CMD_EXISTS:
				break;
			case CHAN_CMD_INTSPEC:
				//*r_arg = chan->int_spec;
				break;
			case CHAN_CMD_ALLOC:
				// all units always working with CPU 0
				*r_arg = 0;
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
				//chan->int_mask = 1;
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
		return IO_NO;
	}
}

// vim: tabstop=4 shiftwidth=4 autoindent
