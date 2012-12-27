//  Copyright (c) 2012 Jakub Filipowicz <jakubf@gmail.com>
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

#include "errors.h"
#include "io.h"
#include "memory.h"
#include "registers.h"

#include "drv/drivers.h"

struct chan_t io_chan[IO_MAX_CHAN];
struct unit_t io_unit[IO_MAX_CHAN][IO_MAX_UNIT];

int io_chan_conf[IO_MAX_CHAN] = { 0, CHAN_MEM, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
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
int io_init()
{
	for (int i=0 ; i<IO_MAX_CHAN ; i++) {
		int ctype = io_chan_conf[i];

		// common channel initialization
		io_chan[i].type = ctype;
		io_chan[i].finish = 0;
		io_chan[i].f_shutdown = drv_chan[ctype].f_shutdown;
		io_chan[i].f_reset = drv_chan[ctype].f_reset;
		io_chan[i].f_cmd = drv_chan[ctype].f_cmd;

		// initialize the channel
		drv_chan[ctype].f_init(io_chan+i);

		// initialize all units connected
		for (int j=0 ; j<IO_MAX_UNIT ; j++) {
			int utype = io_unit_conf[i][j];
			io_unit[i][j].chan = io_chan + i;

			// check if unit can be connected to this type of channel
			if ((ctype == drv_unit[utype].chan_type) || (utype == UNIT_NONE)) {
			} else {
				return E_IO_INCOMPATIBILE_UNIT;
			}
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
