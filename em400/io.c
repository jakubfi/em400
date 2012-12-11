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

#include "io.h"
#include "memory.h"
#include "utils.h"

// -----------------------------------------------------------------------
int io_dispatch(int dir, int is_mem, int chan, int unit, int cmd, uint16_t arg)
{
	if (is_mem) {
		if (dir != IO_OU) {
			// TODO: what to return, really?
			return IO_NO;
		} else {
			// software memory configuration
			int nb = arg & 0b0000000000001111;
			int ab = (arg & 0b1111000000000000) >> 12;
			// here, channel is memory module, unit is memory segment
			//printf("I/O OU: memory nb:%i ab:%i mp:%i seg:%i\n", nb, ab, chan, unit);
			return mem_add_map(nb, ab, chan, unit);
		}
	} else {
		// command to channel or device control unit
		// N - information for selected channel/unit
		char* s_cmd = int2bin(cmd, 8);
		char* s_inf = int2bin(arg, 16);
		//printf("I/O OU: device chan:%i unit:%i command:%s information:%s\n", chan, unit, s_cmd, s_inf);
		free(s_cmd);
		free(s_inf);
		// no such channel/unit
		return IO_NO;
	}
}

// vim: tabstop=4
