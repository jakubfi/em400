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
#include <strings.h>

#include "io/io.h"
#include "io/chan.h"
#include "io/cchar.h"
#include "io/cchar_term_cons.h"

#include "cfg.h"
#include "errors.h"

#include "debugger/log.h"

#define CHAN ((struct cchar_chan_t *)(chan))

// unit prototypes
struct cchar_unit_proto_t cchar_unit_proto[] = {
	{
		"term_cons",
		cchar_term_cons_create,
		cchar_term_cons_shutdown,
		cchar_term_cons_reset,
		cchar_term_cons_cmd
	},
	{ NULL, NULL, NULL, NULL, NULL }
};

// -----------------------------------------------------------------------
struct cchar_unit_proto_t * cchar_unit_proto_get(struct cchar_unit_proto_t *proto, char *name)
{
	while (proto && proto->name) {
		if (strcasecmp(name, proto->name) == 0) {
			return proto;
		}
		proto++;
	}
	return NULL;
}

// -----------------------------------------------------------------------
struct chan_proto_t * cchar_create(struct cfg_unit_t *units)
{
	struct cchar_chan_t *chan = calloc(1, sizeof(struct cchar_chan_t));
	struct cfg_unit_t *cunit = units;
	while (cunit) {
		// find unit prototype
		struct cchar_unit_proto_t *proto = cchar_unit_proto_get(cchar_unit_proto, cunit->name);
		if (!proto) {
			gerr = E_IO_UNIT_UNKNOWN;
			free(chan);
			return NULL;
		}
		eprint("    Unit %i: %s\n", cunit->num, proto->name);

		// create unit based on prototype
		struct cchar_unit_proto_t *unit = proto->create(cunit->args);
		if (!unit) {
			free(chan);
			return NULL;
		}

		// fill in functions
		unit->name = proto->name;
		unit->create = proto->create;
		unit->shutdown = proto->shutdown;
		unit->reset = proto->reset;
		unit->cmd = proto->cmd;

		// remember the channel unit is connected to
		unit->chan = chan;
		unit->num = cunit->num;

		CHAN->unit[unit->num] = unit;

		cunit = cunit->next;
	}

	return (struct chan_proto_t *) chan;
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
int cchar_cmd_intspec(struct chan_proto_t *chan, uint16_t *r_arg)
{
	LOG(D_IO, 1, "CCHAR (ch:%i) command: intspec", chan->num);
	return IO_OK;
}

// -----------------------------------------------------------------------
int cchar_chan_cmd(struct chan_proto_t *chan, int dir, int cmd, int u_num, uint16_t *r_arg)
{
	if (dir == IO_OU) {
		switch (cmd) {
		case CHAN_CMD_EXISTS:
			LOG(D_IO, 1, "CCHAR %i (%s): command: check chan exists", chan->num, chan->name);
			break;
		case CHAN_CMD_INTSPEC:
			return cchar_cmd_intspec(chan, r_arg);
		case CHAN_CMD_ALLOC:
			// all units always working with CPU 0
			*r_arg = 0;
			LOG(D_IO, 1, "CCHAR %i:%i (%s): command: get allocation -> %i", chan->num, u_num, chan->name, *r_arg);
			break;
		default:
			LOG(D_IO, 1, "CCHAR %i:%i (%s): unknow command", chan->num, u_num, chan->name);
			// shouldn't happen, but as channel always reports OK...
			break;
		}
	} else {
		switch (cmd) {
		case CHAN_CMD_EXISTS:
			LOG(D_IO, 1, "CCHAR %i (%s): command: check chan exists", chan->num, chan->name);
			break;
		case CHAN_CMD_MASK_PN:
			CHAN->int_mask = 1;
			LOG(D_IO, 1, "CCHAR %i (%s): command: mask CPU", chan->num, chan->name);
			break;
		case CHAN_CMD_MASK_NPN:
			LOG(D_IO, 1, "CCHAR %i (%s): command: mask ~CPU -> ignored", chan->num, chan->name);
			// ignore 2nd CPU
			break;
		case CHAN_CMD_ASSIGN:
			LOG(D_IO, 1, "CCHAR %i (%s:%s): command: assign CPU -> ignored", chan->num, u_num, chan->name);
			// always for CPU 0
			break;
		default:
			LOG(D_IO, 1, "CCHAR %i:%i (%s): unknow command", chan->num, u_num, chan->name);
			// shouldn't happen, but as channel always reports OK...
			break;
		}
	}
	return IO_OK;
}

// -----------------------------------------------------------------------
int cchar_cmd(struct chan_proto_t *chan, int dir, uint16_t n_arg, uint16_t *r_arg)
{
	int cmd		= (n_arg & 0b1111110000000000) >> 10;
	int u_num   = (n_arg & 0b0000000011100000) >> 5;

	if ((cmd & 0b111000) == 0) { // command for channel
		return cchar_chan_cmd(chan, dir, cmd, u_num, r_arg);
	} else { // command for unit
		if (CHAN->unit[u_num]) {
			return CHAN->unit[u_num]->cmd(CHAN->unit[u_num], dir, cmd, r_arg);
		} else {
			return IO_NO;
		}
	}
}

// vim: tabstop=4 shiftwidth=4 autoindent
