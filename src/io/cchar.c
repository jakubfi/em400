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

#include <stdlib.h>
#include <inttypes.h>
#include <pthread.h>
#include <unistd.h>
#include <strings.h>

#include "io/io.h"
#include "io/chan.h"
#include "io/cchar.h"
#include "io/cchar_term.h"

#include "cfg.h"
#include "log.h"

// unit prototypes
struct cchar_unit_proto_t cchar_unit_proto[] = {
	{
		"terminal",
		cchar_term_create,
		cchar_term_shutdown,
		cchar_term_reset,
		cchar_term_cmd
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
void * cchar_create(int num, struct cfg_unit *units)
{
	struct cchar_chan_t *chan = calloc(1, sizeof(struct cchar_chan_t));

	chan->num = num;

	struct cfg_unit *cunit = units;
	while (cunit) {
		// find unit prototype
		struct cchar_unit_proto_t *proto = cchar_unit_proto_get(cchar_unit_proto, cunit->name);
		if (!proto) {
			LOGERR("Unknown device type or device incompatibile with channel: %s.", cunit->name);
			free(chan);
			return NULL;
		}
		LOG(L_CCHR, 1, "Adding unit %i: %s", cunit->num, proto->name);

		// create unit based on prototype
		struct cchar_unit_proto_t *unit = proto->create(cunit->args);
		if (!unit) {
			LOGERR("Failed to create unit: %s.", cunit->name);
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

		chan->unit[unit->num] = unit;

		cunit = cunit->next;
	}

	// clear unit interrupts
	pthread_mutex_lock(&chan->int_mutex);
	for (int unit_n=0 ; unit_n<CCHAR_MAX_DEVICES ; unit_n++) {
		chan->int_unit[unit_n] = CCHAR_INT_NONE;
	}
	chan->int_reported = -1;
	pthread_mutex_unlock(&chan->int_mutex);

	return (void *) chan;
}

// -----------------------------------------------------------------------
void cchar_shutdown(void *chan)
{
}

// -----------------------------------------------------------------------
void cchar_reset(void *chan)
{
}

// -----------------------------------------------------------------------
void cchar_int_report(struct cchar_chan_t *chan)
{
	// if interrupt is reported and not yet served, there's nothing to do, for now
	pthread_mutex_lock(&chan->int_mutex);
	if (chan->int_reported != -1) {
		pthread_mutex_unlock(&chan->int_mutex);
		return;
	}
	pthread_mutex_unlock(&chan->int_mutex);

	// check if any unit reported interrupt
	for (int unit_n=0 ; unit_n<CCHAR_MAX_DEVICES ; unit_n++) {
		pthread_mutex_lock(&chan->int_mutex);
		if ((chan->int_unit[unit_n] != CCHAR_INT_NONE) && !chan->int_mask) {
			chan->int_reported = unit_n;
			pthread_mutex_unlock(&chan->int_mutex);
			LOG(L_CCHR, 3, "CCHAR (ch:%i) reporting interrupt %i", chan->num, chan->num + 12);
			io_int_set(chan->num);
			break;
		} else {
			pthread_mutex_unlock(&chan->int_mutex);
		}
	}
}

// -----------------------------------------------------------------------
void cchar_int(struct cchar_chan_t *chan, int unit_n, int interrupt)
{
	LOG(L_CCHR, 1, "CCHAR (ch:%i) interrupt %i, unit: %i", chan->num, interrupt, unit_n);
	pthread_mutex_lock(&chan->int_mutex);
	chan->int_unit[unit_n] = interrupt;
	pthread_mutex_unlock(&chan->int_mutex);

	cchar_int_report(chan);
}

// -----------------------------------------------------------------------
int cchar_cmd_intspec(struct cchar_chan_t *chan, uint16_t *r_arg)
{
	LOG(L_CCHR, 1, "CCHAR (ch:%i) command: intspec", chan->num);
	pthread_mutex_lock(&chan->int_mutex);
	if (chan->int_reported != -1) {
		*r_arg = (chan->int_unit[chan->int_reported] << 8) | (chan->int_reported << 5);
		// mark interrupt as served
		chan->int_unit[chan->int_reported] = CCHAR_INT_NONE;
		// nothing new reported
		chan->int_reported = -1;
	} else {
		*r_arg = 0;
	}
	pthread_mutex_unlock(&chan->int_mutex);

	// report another interrupt if it's there
	cchar_int_report(chan);

	return IO_OK;
}


// -----------------------------------------------------------------------
int cchar_chan_cmd(struct cchar_chan_t *chan, int dir, int cmd, int u_num, uint16_t *r_arg)
{
	if (dir == IO_OU) {
		switch (cmd) {
		case CHAN_CMD_EXISTS:
			LOG(L_CCHR, 1, "CCHAR %i: command: check chan exists", chan->num);
			break;
		case CHAN_CMD_MASK_PN:
			chan->int_mask = 1;
			LOG(L_CCHR, 1, "CCHAR %i: command: mask CPU", chan->num);
			break;
		case CHAN_CMD_MASK_NPN:
			LOG(L_CCHR, 1, "CCHAR %i: command: mask ~CPU -> ignored", chan->num);
			// ignore 2nd CPU
			break;
		case CHAN_CMD_ASSIGN:
			LOG(L_CCHR, 1, "CCHAR %i:%i: command: assign CPU -> ignored", chan->num, u_num);
			// always for CPU 0
			break;
		default:
			LOG(L_CCHR, 1, "CCHAR %i:%i: unknow command", chan->num, u_num);
			// shouldn't happen, but as channel always reports OK...
			break;
		}
	} else {
		switch (cmd) {
		case CHAN_CMD_EXISTS:
		case CHAN_CMD_STATUS:
			LOG(L_CCHR, 1, "CCHAR %i: command: check chan exists", chan->num);
			break;
		case CHAN_CMD_INTSPEC:
			return cchar_cmd_intspec(chan, r_arg);
		case CHAN_CMD_ALLOC:
			// all units always working with CPU 0
			*r_arg = 0;
			LOG(L_CCHR, 1, "CCHAR %i:%i: command: get allocation -> %i", chan->num, u_num, *r_arg);
			break;
		default:
			LOG(L_CCHR, 1, "CCHAR %i:%i: unknow command", chan->num, u_num);
			// shouldn't happen, but as channel always reports OK...
			break;
		}
	}
	return IO_OK;
}

// -----------------------------------------------------------------------
int cchar_cmd(void *ch, int dir, uint16_t n_arg, uint16_t *r_arg)
{
	int cmd		= (n_arg & 0b1111110000000000) >> 10;
	int u_num   = (n_arg & 0b0000000011100000) >> 5;

	struct cchar_chan_t *chan = (struct cchar_chan_t *) ch;

	if ((cmd & 0b111000) == 0) { // command for channel
		return cchar_chan_cmd(chan, dir, cmd, u_num, r_arg);
	} else { // command for unit
		if (chan->unit[u_num]) {
			return chan->unit[u_num]->cmd(chan->unit[u_num], dir, cmd, r_arg);
		} else {
			return IO_NO;
		}
	}
}

struct chan_drv cchar_chan_driver = {
	.name = "char",
	.create = cchar_create,
	.shutdown = cchar_shutdown,
	.reset = cchar_reset,
	.cmd = cchar_cmd
};

// vim: tabstop=4 shiftwidth=4 autoindent
