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

#include "utils.h"
#include "errors.h"
#include "io.h"
#include "memory.h"
#include "registers.h"

#include "drv/drivers.h"

#include "debugger/log.h"

struct chan_t io_chan[IO_MAX_CHAN];
struct unit_t io_unit[IO_MAX_CHAN][IO_MAX_UNIT];

int io_chan_conf[IO_MAX_CHAN] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int io_unit_conf[IO_MAX_CHAN][IO_MAX_UNIT] = {
{ 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0 }
};

// -----------------------------------------------------------------------
int io_chan_init(struct chan_t *chan, int ctype)
{
	// channel driver description
	struct drv_chan_t *drv = drv_chan + ctype;

	// driver sanity checks
	if ((!drv) || (!drv->f_init) || (!drv->f_shutdown) || (!drv->f_reset) || (!drv->f_cmd) || (ctype != drv->type)) {
		return E_IO_DRV_CHAN_BAD;
	}

	// common channel initialization
	chan->type = drv->type;
	chan->name = drv->name;
	chan->finish = 0;
	chan->f_shutdown = drv->f_shutdown;
	chan->f_reset = drv->f_reset;
	chan->f_cmd = drv->f_cmd;

	// initialize the channel
	drv->f_init(chan);

	return E_OK;
}

// -----------------------------------------------------------------------
int io_unit_init(struct unit_t *unit, int utype)
{
	// unit driver description
	struct drv_unit_t *drv = drv_unit + utype;

	// driver sanity checks
	if ((!drv) || (!drv->f_init) || (!drv->f_shutdown) || (!drv->f_reset) || (!drv->f_cmd) || (utype != drv->type)) {
		return E_IO_DRV_UNIT_BAD;
	}

	unit->type = drv->type;
	unit->name = drv->name;

	// check if unit can be connected to this type of channel
	if ((unit->chan->type == drv->chan_type) || (unit->type == UNIT_NONE)) {
		unit->f_shutdown = drv->f_shutdown;
		unit->f_reset = drv->f_reset;
		unit->f_cmd = drv->f_cmd;
	} else {
		// unit driver is incompatibile with this type of channel
		return E_IO_INCOMPATIBILE_UNIT;
	}

	return E_OK;
}

// -----------------------------------------------------------------------
int io_init()
{
	// initialize all channels
	for (int i=0 ; i<IO_MAX_CHAN ; i++) {
		io_chan[i].number = i;
		io_chan_init(io_chan+i, io_chan_conf[i]);

		// initialize all units connected to each channel
		for (int j=0 ; j<IO_MAX_UNIT ; j++) {
			io_unit[i][j].number = j;
			io_unit[i][j].chan = io_chan+i;
			io_unit_init(io_unit[i]+j, io_unit_conf[i][j]);
		}
	}
	return E_OK;
}

// -----------------------------------------------------------------------
void io_shutdown()
{
	for (int i=0 ; i<IO_MAX_CHAN ; i++) {
		for (int j=0 ; j<IO_MAX_UNIT ; j++) {
		}
	}
}

// -----------------------------------------------------------------------
uint16_t io_get_int_spec(int interrupt)
{
	// TODO: check if the interrupt hasn't been masked in the meantime
	int chan = interrupt - 12;
	return io_chan[chan].int_spec;
}

// -----------------------------------------------------------------------
int io_dispatch(int dir, uint16_t n, int r)
{
	int is_mem = (n & 0b0000000000000001);
	int chan = (n & 0b0000000000011110) >> 1;
	int unit = (n & 0b0000000011100000) >> 5;
	int cmd = (n & 0b1111111100000000) >> 8;

	// software memory configuration
	if (is_mem) {
		LOG(D_IO, 1, "MEM command, dir = %s, module = %d, segment = %d, cmd = %d, r = %d", dir ? "OUT" : "IN", chan, unit, cmd, r);
		if (dir == IO_OU) {
			int nb = R(r) & 0b0000000000001111;
			int ab = (R(r) & 0b1111000000000000) >> 12;
			// here, channel is memory module, unit is memory segment
			return mem_add_map(nb, ab, chan, unit);
		} else {
			// TODO: what to return?
			return IO_NO;
		}
	// channel/unit command
	} else {
#ifdef WITH_DEBUGGER
		char *cmdc = int2bin(cmd, 8);
		LOG(D_IO, 1, "I/O command, dir = %s, chan = %d, unit = %d, cmd = %s, r = %d", dir ? "OUT" : "IN", chan, unit, cmdc, r);
		free(cmdc);
#endif
		// three most sig. bits are 0 if this is channel command
		if ((n & 0b1110000000000000) == 0) {
			return io_chan[chan].f_cmd(io_chan+chan, dir, unit, cmd, r);
		// unit command
		} else {
			return io_unit[chan][unit].f_cmd(io_unit[chan]+unit, dir, cmd, r);
		}
	}
}

// vim: tabstop=4
