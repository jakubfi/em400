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
#include <string.h>
#include <strings.h>

#include "cpu/memory.h"
#include "cpu/registers.h"
#include "io/io.h"
#include "io/cmem.h"
#include "io/cchar.h"
#include "io/multix.h"
#include "io/plix.h"

#include "cfg.h"
#include "utils.h"
#include "errors.h"

#include "debugger/log.h"

struct fundict_t chan_init[] = {
	{ "char",		cchar_init },
	{ "mem",		cmem_init },
	{ "multix",		multix_init },
	{ "plix",		plix_init },
	{ NULL,			NULL }
};

struct chan_t *io_chan[IO_MAX_CHAN];

// -----------------------------------------------------------------------
chan_initfun io_chan_getinit(char *name)
{
	struct fundict_t *ci = chan_init;
	while (ci->name) {
		if (strcasecmp(name, ci->name) == 0) {
			return ci->f_init;
		}
		ci++;
	}
	return NULL;
}

// -----------------------------------------------------------------------
int io_chan_init(int num, char *name, struct cfg_unit_t *units)
{
	chan_initfun chan_init = io_chan_getinit(name);
	if (!chan_init) {
		return E_IO_CHAN_UNKNOWN;
	}

	eprint("  Channel %i: %s\n", num, name);

	struct chan_t *chan = calloc(1, sizeof(struct chan_t));
	if (!chan) {
		return E_ALLOC;
	}

	chan->name = strdup(name);
	chan->num = num;
	io_chan[num] = chan;

	chan_init(chan, units);

	return E_OK;
}

// -----------------------------------------------------------------------
int io_init()
{
	int res;
	struct cfg_chan_t *chanc = em400_cfg.chans;

	eprint("Initializing I/O\n");

	while (chanc) {
		res = io_chan_init(chanc->num, chanc->name, chanc->units);
		if (res != E_OK) {
			return res;
		}
		chanc = chanc->next;
	}

	cfg_drop_chans(em400_cfg.chans);

	return E_OK;
}

// -----------------------------------------------------------------------
void io_shutdown()
{
	eprint("Shutdown I/O\n");
	for (int c_num=0 ; c_num<IO_MAX_CHAN ; c_num++) {
		struct chan_t *chan = io_chan[c_num];
		if (chan) {
			eprint("    Shutdown channel %i (%s)\n", chan->num, chan->name);
			chan->f_shutdown(chan);
			free(chan);
		}
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
		int chan_n = (n & 0b0000000000011110) >> 1;
		struct chan_t *chan = io_chan[chan_n];
		int res;
		if (chan) {
#ifdef WITH_DEBUGGER
			char *narg = int2bin(n, 16);
			char *rarg = int2bin(*r, 16);
			LOG(D_IO, 1, "I/O command, dir = %s, chan = %d, n_arg = %s, r_arg = %s", dir ? "OUT" : "IN", chan_n, narg, rarg);
			free(narg);
			free(rarg);
#endif
			res = chan->f_cmd(chan, dir, n, r);
			LOG(D_IO, 1, "I/O command, result = %i", res);
		} else {
			res = IO_NO;
			LOG(D_IO, 1, "I/O command to a channel that doesn't exist: %i", chan_n);
		}
		return res;
	}
}

// vim: tabstop=4 shiftwidth=4 autoindent
