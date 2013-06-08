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
#include "io/cmem.h"

#include "cfg.h"
#include "errors.h"

#include "debugger/log.h"

// -----------------------------------------------------------------------
int cmem_init(struct chan_t *chan, struct cfg_unit_t *units)
{
	chan->f_shutdown = cmem_shutdown;
	chan->f_reset = cmem_reset;
	chan->f_cmd = cmem_cmd;
	cmem_reset(chan);
	return E_OK;
}

// -----------------------------------------------------------------------
void cmem_shutdown(struct chan_t *chan)
{
}

// -----------------------------------------------------------------------
void cmem_reset(struct chan_t *chan)
{
}

// -----------------------------------------------------------------------
int cmem_cmd(struct chan_t *chan, int dir, uint16_t n_arg, uint16_t *r_arg)
{
	int u_num = (n_arg & 0b0000000011100000) >> 5;
	int cmd = (n_arg & 0b1111111100000000) >> 8;

	//chan->int_mask = 0;

	// command for channel
	if ((cmd & 0b11100000) == 0) {
		if (dir == IO_OU) {
			switch (cmd & 0b00011000) {
			case CHAN_CMD_EXISTS:
				LOG(D_IO, 1, "CMEM %i (%s): CHAN_CMD_EXISTS", chan->num, chan->name);
				break;
			case CHAN_CMD_INTSPEC:
				//*r_arg = chan->int_spec;
				LOG(D_IO, 1, "CMEM %i (%s): CHAN_CMD_INTSPEC -> %i", chan->num, chan->name, *r_arg);
				break;
			case CHAN_CMD_STATUS:
				//*r_arg = chan->untransmitted;
				LOG(D_IO, 1, "CMEM %i (%s) CHAN_CMD_STATUS", chan->num, chan->name);
				break;
			case CHAN_CMD_ALLOC:
				// all units always working with CPU 0
				*r_arg = 0;
				LOG(D_IO, 1, "CMEM %i:%i (%s): CHAN_CMD_ALLOC -> %i", chan->num, u_num, chan->name, *r_arg);
				break;
			default:
				// shouldn't happen, but as channel always reports OK...
				break;
			}
		} else {
			switch (cmd & 0b00011000) {
			case CHAN_CMD_EXISTS:
				LOG(D_IO, 1, "CMEM %i (%s): CHAN_CMD_EXISTS", chan->num, chan->name);
				break;
			case CHAN_CMD_MASK_PN:
				//chan->int_mask = 1;
				LOG(D_IO, 1, "CMEM %i (%s): CHAN_CMD_MASK_PN", chan->num, chan->name);
				break;
			case CHAN_CMD_MASK_NPN:
				LOG(D_IO, 1, "CMEM %i (%s): CHAN_CMD_MASK_NPN -> ignored", chan->num, chan->name);
				// ignore 2nd CPU
				break;
			case CHAN_CMD_ASSIGN:
				LOG(D_IO, 1, "CMEM %i (%s:%s): CHAN_CMD_ASSIGN -> ignored", chan->num, u_num, chan->name);
				// always for CPU 0
				break;
			default:
				LOG(D_IO, 1, "CMEM %i:%i (%s): unknow command", chan->num, u_num, chan->name);
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