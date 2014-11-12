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

#include "mem/mem.h"
#include "cpu/cpu.h"
#include "io/io.h"
#include "io/cmem.h"
#include "io/cchar.h"
#include "io/multix.h"
#include "io/plix.h"

#include "cfg.h"
#include "utils.h"
#include "errors.h"

#include "log.h"

static struct chan_proto_t chan_proto[] = {
	{ -1, "char",		cchar_create,	cchar_shutdown,	cchar_reset,	cchar_cmd },
	{ -1, "mem",		cmem_create,	cmem_shutdown,	cmem_reset,		cmem_cmd },
	{ -1, "multix",		mx_create,		mx_shutdown,	mx_reset,		mx_cmd },
	{ -1, "plix",		px_create,		NULL,			NULL,			NULL },
	{ -1, NULL,			NULL,			NULL,			NULL,			NULL }
};

struct chan_proto_t *io_chan[IO_MAX_CHAN];

static const char *io_result_names[] = {
	"NO DEVICE",
	"ENGAGED",
	"OK",
	"PARITY ERROR"
};

// -----------------------------------------------------------------------
static struct chan_proto_t * io_chan_maker(int num, char *name, struct cfg_unit *units)
{
	struct chan_proto_t *proto = chan_proto;
	struct chan_proto_t *chan = NULL;

	while (proto && proto->name) {
		if (!strcasecmp(name, proto->name)) {
			chan = proto->create(units);
			break;
		}
		proto++;
	}

	if (chan) {
		chan->num = num;
		chan->name = proto->name;
		chan->create = proto->create;
		chan->shutdown = proto->shutdown;
		chan->reset = proto->reset;
		chan->cmd = proto->cmd;
	} else if (!proto->name) {
		gerr = E_IO_CHAN_UNKNOWN;
	}

	return chan;
}

// -----------------------------------------------------------------------
int io_init(struct cfg_em400 *cfg)
{
	struct cfg_chan *chanc = cfg->chans;
	struct chan_proto_t *chan;

	LOG(L_IO, 1, "Initializing I/O");

	while (chanc) {
		LOG(L_IO, 1, "Channel %i: %s", chanc->num, chanc->name);
		chan = io_chan_maker(chanc->num, chanc->name, chanc->units);
		if (!chan) {
			return gerr;
		} else {
			chan->reset(chan);
			io_chan[chanc->num] = chan;
		}
		chanc = chanc->next;
	}

	return E_OK;
}

// -----------------------------------------------------------------------
void io_shutdown()
{
	LOG(L_IO, 1, "Shutdown I/O");
	for (int c_num=0 ; c_num<IO_MAX_CHAN ; c_num++) {
		struct chan_proto_t *chan = io_chan[c_num];
		if (chan) {
			LOG(L_IO, 1, "Channel %i: %s", chan->num, chan->name);
			chan->shutdown(chan);
			io_chan[c_num] = NULL;
		}
	}
}

// -----------------------------------------------------------------------
void io_reset()
{
	for (int c_num=0 ; c_num<IO_MAX_CHAN ; c_num++) {
		struct chan_proto_t *chan = io_chan[c_num];
		if (chan) {
			chan->reset(chan);
		}
	}
}

// -----------------------------------------------------------------------
int io_dispatch(int dir, uint16_t n, uint16_t *r)
{
	int is_mem_cmd = n & 1; // 1 = memory configuration, 0 = I/O command

	// software memory configuration
	if (is_mem_cmd) {
		if (dir == IO_OU) {
			return mem_cmd(n, *r);
		} else {
			LOG(L_IO, 1, "MEM command shouldn't be IN");
			// TODO: what to return?
			return IO_NO;
		}
	// channel/unit command
	} else {
		int chan_n = (n & 0b0000000000011110) >> 1;
		struct chan_proto_t *chan = io_chan[chan_n];
		int res;
		if (LOG_WANTS(L_IO, 1)) {
			char *narg = int2binf("cmd: ... .. ...... ch: .... .", n, 16);
			LOG(L_IO, 1, "I/O %s chan = %d, n_arg = %s (0x%04x), r_arg = 0x%04x", dir ? "IN" : "OUT", chan_n, narg, n, *r);
			free(narg);
		}

		if (chan) {
			res = chan->cmd(chan, dir, n, r);
		} else {
			res = IO_NO;
		}
		LOG(L_IO, 1, "I/O result: %s, r_arg = 0x%04x", io_result_names[res], *r);
		return res;
	}
}

// vim: tabstop=4 shiftwidth=4 autoindent
