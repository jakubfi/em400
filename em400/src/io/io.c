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
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>

#include "cpu/memory.h"
#include "cpu/registers.h"
#include "io/io.h"
#include "io/drivers.h"

#include "cfg.h"
#include "utils.h"
#include "errors.h"

#include "debugger/log.h"

struct chan_t *io_chan[IO_MAX_CHAN];

// -----------------------------------------------------------------------
int io_chan_init(int c_num)
{
	struct drv_t *c_driver;
	if (em400_cfg.chans[c_num].name) {
		c_driver = drv_get(DRV_CHAN, CHAN_IGNORE, em400_cfg.chans[c_num].name);
		free(em400_cfg.chans[c_num].name);
	} else {
		c_driver = drv_get(DRV_CHAN, CHAN_IGNORE, "none");
	}

	if (!c_driver) {
		return E_IO_CHAN_UNKNOWN;
	} 

	// driver sanity checks
	if ((!c_driver->f_init) || (!c_driver->f_shutdown) || (!c_driver->f_reset) || (!c_driver->f_cmd)) {
		return E_IO_DRV_CHAN_BAD;
	}

	struct chan_t *chan = malloc(sizeof(struct chan_t));
	io_chan[c_num] = chan;

	// common channel initialization
	chan->unit = calloc(c_driver->max_devs, sizeof(struct unit_t*));
	chan->num = c_num;
	chan->type = c_driver->chan_type;
	chan->name = c_driver->name;
	chan->max_devs = c_driver->max_devs;
	chan->finish = 0;
	chan->f_shutdown = c_driver->f_shutdown;
	chan->f_reset = c_driver->f_reset;
	chan->f_cmd = c_driver->f_cmd;

	if (em400_cfg.chans[c_num].name) {
		eprint("  Channel %i (%s):\n", chan->num, chan->name);
	}

	// initialize the channel
	return c_driver->f_init(chan, NULL);
}

// -----------------------------------------------------------------------
int io_unit_init(int c_num, int u_num)
{
	struct drv_t *u_driver;
	if (em400_cfg.chans[c_num].units[u_num].name) {
		u_driver = drv_get(DRV_UNIT, io_chan[c_num]->type, em400_cfg.chans[c_num].units[u_num].name);
		free(em400_cfg.chans[c_num].units[u_num].name);
		if (!u_driver) {
			return E_IO_UNIT_UNKNOWN;
		}
	} else {
		u_driver = NULL;
		return E_OK;
	}

	// driver sanity checks
	if ((!u_driver->f_init) || (!u_driver->f_shutdown) || (!u_driver->f_reset) || (!u_driver->f_cmd)) {
		return E_IO_DRV_UNIT_BAD;
	}

	struct unit_t *unit = malloc(sizeof(struct unit_t));
	io_chan[c_num]->unit[u_num] = unit;

	unit->num = u_num;
	unit->name = u_driver->name;
	unit->chan = io_chan[c_num];
	unit->cfg = NULL;
	unit->f_shutdown = u_driver->f_shutdown;
	unit->f_reset = u_driver->f_reset;
	unit->f_cmd = u_driver->f_cmd;

	if (em400_cfg.chans[c_num].units[u_num].name) {
		eprint("          %i:%i (%s)\n", c_num, unit->num, unit->name);
	}

	// initialize the unit
	return u_driver->f_init(unit, em400_cfg.chans[c_num].units[u_num].args);
}

// -----------------------------------------------------------------------
int io_init()
{
	int res;

	eprint("Initializing I/O\n");

	for (int c_num=0 ; c_num<IO_MAX_CHAN ; c_num++) {
		// initialize channel
		res = io_chan_init(c_num);
		if (res != E_OK) {
			return res;
		}

		// initialize units connected
		for (int u_num=0 ; u_num<io_chan[c_num]->max_devs ; u_num++) {
			res = io_unit_init(c_num, u_num);
			if (res != E_OK) {
				return res;
			}
		}
	}
	return E_OK;
}

// -----------------------------------------------------------------------
void io_shutdown()
{
	eprint("Shutdown I/O\n");
	for (int c_num=0 ; c_num<IO_MAX_CHAN ; c_num++) {
		struct chan_t *chan = io_chan[c_num];
		if (!chan) {
			continue;
		}

		for (int u_num=0 ; u_num<chan->max_devs ; u_num++) {
			struct unit_t *unit = chan->unit[u_num];
			if (!unit) {
				continue;
			}

			eprint("  Shutdown unit %i (%s)\n", u_num, unit->name);
			unit->f_shutdown(unit);
			free(unit);
		}

		eprint("    Shutdown channel %i (%s)\n", c_num, chan->name);
		chan->f_shutdown(chan);
		free(chan->unit);
		free(chan);
	}
}

// -----------------------------------------------------------------------
int io_dispatch(int dir, uint16_t n, uint16_t *r)
{
	int is_mem = (n & 0b0000000000000001);		// 1 = mem config

	// software memory configuration
	if (is_mem) {
		if (dir == IO_OU) {
			LOG(D_IO, 1, "MEM command");
			int nb = *r & 0b0000000000001111;
			int ab = (*r & 0b1111000000000000) >> 12;
			int module = (n & 0b0000000000011110) >> 1;
			int seg = (n & 0b0000000111100000) >> 5;
			return mem_add_map(nb, ab, module, seg);
		} else {
			// TODO: what to return?
			LOG(D_IO, 1, "MEM command shouldn't be IN");
			return IO_NO;
		}
	// channel/unit command
	} else {
		int chan = (n & 0b0000000000011110) >> 1;
#ifdef WITH_DEBUGGER
		char *narg = int2bin(n, 16);
		char *rarg = int2bin(*r, 16);
		LOG(D_IO, 1, "I/O command, dir = %s, chan = %d, n_arg = %s, r_arg = %s", dir ? "OUT" : "IN", chan, narg, rarg);
		free(narg);
		free(rarg);
#endif
		int res = io_chan[chan]->f_cmd(io_chan[chan], dir, n, r);
		LOG(D_IO, 1, "I/O command, result = %i", res);
		return res;
	}
}

// vim: tabstop=4
