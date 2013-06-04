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
void * drv_cchar_thread(void *self)
{
	struct chan_t *ch = self;

	struct timespec ts;
/**/	ts.tv_sec = 0;
/**/	ts.tv_nsec = 1000000;

	while (!ch->finish) {
/**/		nanosleep(&ts, &ts);
	}
	pthread_exit(NULL);
}

// -----------------------------------------------------------------------
int drv_cchar_init(void *self, struct cfg_arg_t *arg)
{
	struct chan_t *ch = self;
	drv_cchar_reset(ch);
	pthread_create(&ch->thread, NULL, drv_cchar_thread, ch);
	return E_OK;
}

// -----------------------------------------------------------------------
void drv_cchar_shutdown(void *self)
{
	struct chan_t *ch = self;
	ch->finish = 1;
	pthread_join(ch->thread, NULL);
}

// -----------------------------------------------------------------------
void drv_cchar_reset(void *self)
{
	struct chan_t *ch = self;
	ch->int_spec = 0;
	ch->int_mask = 0;
}

// -----------------------------------------------------------------------
int drv_cchar_cmd(void *self, int dir, uint16_t n_arg, uint16_t *r_arg)
{
	struct chan_t *ch = self;

	int u_num = (n_arg & 0b0000000011100000) >> 5;
	int cmd = (n_arg & 0b1111111100000000) >> 8;

	struct unit_t *unit = ch->unit[u_num];
	ch->int_mask = 0;

	// command for channel
	if ((cmd & 0b11100000) == 0) {
		if (dir == IO_OU) {
			switch (cmd & 0b00011000) {
			case CHAN_CMD_EXISTS:
				break;
			case CHAN_CMD_INTSPEC:
				*r_arg = ch->int_spec;
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
		return unit->f_cmd(unit, dir, n_arg, r_arg);
	}
}

// vim: tabstop=4 shiftwidth=4 autoindent
