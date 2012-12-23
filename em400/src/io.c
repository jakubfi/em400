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

#include "errors.h"
#include "io.h"
#include "memory.h"
#include "registers.h"

struct chan_t io_channels[IO_MAX_CHAN];

int io_chan_conf[IO_MAX_CHAN] = { CHAN_CHAR, CHAN_MEM, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

// -----------------------------------------------------------------------
int io_init()
{
	for (int i=0 ; i<IO_MAX_CHAN ; i++) {
		io_channels[i].type = io_chan_conf[i];
	}
	return E_OK;
}

// -----------------------------------------------------------------------
void io_shutdown()
{

}

// -----------------------------------------------------------------------
int io_dispatch(int dir, uint16_t n, unsigned short r)
{
	int is_mem = (n & 0b0000000000000001);
	int chan = (n & 0b0000000000011110) >> 1;
	int unit = (n & 0b0000000011100000) >> 5;
	int cmd = (n & 0b1111111100000000) >> 8;

	// software memory configuration
	if (is_mem) {
		// TODO: this should work for OU only, but just in case, do it for IN too
		int nb = R(r) & 0b0000000000001111;
		int ab = (R(r) & 0b1111000000000000) >> 12;
		// here, channel is memory module, unit is memory segment
		return mem_add_map(nb, ab, chan, unit);

	// command for a channel
	} else {
		return IO_NO;
	}
}

// vim: tabstop=4
