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

#include "cpu/interrupts.h"
#include "io/cchar.h"
#include "io/cchar_term.h"

#include "cfg.h"
#include "errors.h"

#include "log.h"

#define CHAN ((struct cchar_chan_t *)(chan))

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
struct chan * cchar_create(struct cfg_unit *units)
{
	struct cchar_chan_t *chan = calloc(1, sizeof(struct cchar_chan_t));
	struct cfg_unit *cunit = units;
	while (cunit) {
		// find unit prototype
		struct cchar_unit_proto_t *proto = cchar_unit_proto_get(cchar_unit_proto, cunit->name);
		if (!proto) {
			gerr = E_IO_UNIT_UNKNOWN;
			free(chan);
			return NULL;
		}
		LOG(L_CCHR, 1, "Adding unit %i: %s", cunit->num, proto->name);

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

	// clear unit interrupts
	pthread_mutex_lock(&chan->int_mutex);
	for (int unit_n=0 ; unit_n<CCHAR_MAX_DEVICES ; unit_n++) {
		CHAN->int_unit[unit_n] = CCHAR_INT_NONE;
	}
	CHAN->int_reported = -1;
	pthread_mutex_unlock(&chan->int_mutex);

	return (struct chan *) chan;
}

// -----------------------------------------------------------------------
void cchar_shutdown(struct chan *chan)
{
}

// -----------------------------------------------------------------------
void cchar_reset(struct chan  *chan)
{
}

// -----------------------------------------------------------------------
void cchar_int_report(struct cchar_chan_t *chan)
{
	// if interrupt is reported and not yet served, there's nothing to do, for now
	pthread_mutex_lock(&CHAN->int_mutex);
	if (CHAN->int_reported != -1) {
		pthread_mutex_unlock(&CHAN->int_mutex);
		return;
	}
	pthread_mutex_unlock(&CHAN->int_mutex);

	// check if any unit reported interrupt
	for (int unit_n=0 ; unit_n<CCHAR_MAX_DEVICES ; unit_n++) {
		pthread_mutex_lock(&CHAN->int_mutex);
		if ((CHAN->int_unit[unit_n] != CCHAR_INT_NONE) && !CHAN->int_mask) {
			chan->int_reported = unit_n;
			pthread_mutex_unlock(&CHAN->int_mutex);
			LOG(L_CCHR, 3, "CCHAR (ch:%i) reporting interrupt %i", chan->proto.num, chan->proto.num + 12);
			int_set(chan->proto.num + 12);
			break;
		} else {
			pthread_mutex_unlock(&CHAN->int_mutex);
		}
	}
}

// -----------------------------------------------------------------------
void cchar_int(struct cchar_chan_t *chan, int unit_n, int interrupt)
{
	LOG(L_CCHR, 1, "CCHAR (ch:%i) interrupt %i, unit: %i", chan->proto.num, interrupt, unit_n);
	pthread_mutex_lock(&CHAN->int_mutex);
	CHAN->int_unit[unit_n] = interrupt;
	pthread_mutex_unlock(&CHAN->int_mutex);

	cchar_int_report(CHAN);
}

// -----------------------------------------------------------------------
int cchar_cmd_intspec(struct chan *chan, uint16_t *r_arg)
{
	LOG(L_CCHR, 1, "CCHAR (ch:%i) command: intspec", chan->num);
	pthread_mutex_lock(&CHAN->int_mutex);
	if (CHAN->int_reported != -1) {
		*r_arg = (CHAN->int_unit[CHAN->int_reported] << 8) | (CHAN->int_reported << 5);
		// mark interrupt as served
		CHAN->int_unit[CHAN->int_reported] = CCHAR_INT_NONE;
		// nothing new reported
		CHAN->int_reported = -1;
	} else {
		*r_arg = 0;
	}
	pthread_mutex_unlock(&CHAN->int_mutex);

	// report another interrupt if it's there
	cchar_int_report(CHAN);

	return IO_OK;
}


// -----------------------------------------------------------------------
int cchar_chan_cmd(struct chan *chan, int dir, int cmd, int u_num, uint16_t *r_arg)
{
	if (dir == IO_OU) {
		switch (cmd) {
		case CHAN_CMD_EXISTS:
			LOG(L_CCHR, 1, "CCHAR %i (%s): command: check chan exists", chan->num, chan->name);
			break;
		case CHAN_CMD_MASK_PN:
			CHAN->int_mask = 1;
			LOG(L_CCHR, 1, "CCHAR %i (%s): command: mask CPU", chan->num, chan->name);
			break;
		case CHAN_CMD_MASK_NPN:
			LOG(L_CCHR, 1, "CCHAR %i (%s): command: mask ~CPU -> ignored", chan->num, chan->name);
			// ignore 2nd CPU
			break;
		case CHAN_CMD_ASSIGN:
			LOG(L_CCHR, 1, "CCHAR %i (%s:%s): command: assign CPU -> ignored", chan->num, u_num, chan->name);
			// always for CPU 0
			break;
		default:
			LOG(L_CCHR, 1, "CCHAR %i:%i (%s): unknow command", chan->num, u_num, chan->name);
			// shouldn't happen, but as channel always reports OK...
			break;
		}
	} else {
		switch (cmd) {
		case CHAN_CMD_EXISTS:
		case CHAN_CMD_STATUS:
			LOG(L_CCHR, 1, "CCHAR %i (%s): command: check chan exists", chan->num, chan->name);
			break;
		case CHAN_CMD_INTSPEC:
			return cchar_cmd_intspec(chan, r_arg);
		case CHAN_CMD_ALLOC:
			// all units always working with CPU 0
			*r_arg = 0;
			LOG(L_CCHR, 1, "CCHAR %i:%i (%s): command: get allocation -> %i", chan->num, u_num, chan->name, *r_arg);
			break;
		default:
			LOG(L_CCHR, 1, "CCHAR %i:%i (%s): unknow command", chan->num, u_num, chan->name);
			// shouldn't happen, but as channel always reports OK...
			break;
		}
	}
	return IO_OK;
}

// -----------------------------------------------------------------------
int cchar_cmd(struct chan *chan, int dir, uint16_t n_arg, uint16_t *r_arg)
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
