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
#include "io/io.h"
#include "io/chan.h"
#include "io/cmem.h"
#include "io/cmem_m9425.h"

#include "cfg.h"
#include "errors.h"

#include "log.h"

#define CHAN ((struct cmem_chan_t *)(chan))

// unit prototypes
struct cmem_unit_proto_t cmem_unit_proto[] = {
	{
		"mera9425",
		cmem_m9425_create,
		cmem_m9425_shutdown,
		cmem_m9425_reset,
		cmem_m9425_cmd
	},
	{ NULL, NULL, NULL, NULL, NULL }
};

// -----------------------------------------------------------------------
struct cmem_unit_proto_t * cmem_unit_proto_get(struct cmem_unit_proto_t *proto, char *name)
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
struct chan * cmem_create(struct cfg_unit *units)
{
	struct cmem_chan_t *chan = calloc(1, sizeof(struct cmem_chan_t));

	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutex_init(&chan->transmit_mutex, &attr);
	pthread_mutex_init(&chan->int_mutex, &attr);

	// initialize units
	struct cfg_unit *cunit = units;
	while (cunit) {
		// find unit prototype
		struct cmem_unit_proto_t *proto = cmem_unit_proto_get(cmem_unit_proto, cunit->name);
		if (!proto) {
			gerr = E_IO_UNIT_UNKNOWN;
			free(chan);
			return NULL;
		}

		LOG(L_CMEM, 1, "Adding unit %i: %s", cunit->num, proto->name);

		// create unit based on prototype
		struct cmem_unit_proto_t *unit = proto->create(cunit->args);
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
	for (int unit_n=0 ; unit_n<CMEM_MAX_DEVICES ; unit_n++) {
		CHAN->int_unit[unit_n] = CMEM_INT_NONE;
	}
	pthread_mutex_unlock(&chan->int_mutex);


	return (struct chan *) chan;
}

// -----------------------------------------------------------------------
void cmem_shutdown(struct chan *chan)
{
	for (int i=0 ; i<CMEM_MAX_DEVICES ; i++) {
		struct cmem_unit_proto_t *unit = CHAN->unit[i];
		if (unit) {
			unit->shutdown(unit);
			CHAN->unit[i] = NULL;
		}
	}
	pthread_mutex_destroy(&CHAN->int_mutex);
	pthread_mutex_destroy(&CHAN->transmit_mutex);
	free(chan);
}

// -----------------------------------------------------------------------
void cmem_reset(struct chan *chan)
{
	LOG(L_CMEM, 1, "CMEM (ch:%i) reset", chan->num);
	for (int i=0 ; i<CMEM_MAX_DEVICES ; i++) {
		struct cmem_unit_proto_t *unit = CHAN->unit[i];
		if (unit) {
			unit->reset(unit);
		}
	}
}

// -----------------------------------------------------------------------
void cmem_int_report(struct cmem_chan_t *chan)
{
	// if interrupt is reported and not yet served, there's nothing to do, for now
	pthread_mutex_lock(&CHAN->int_mutex);
	if (CHAN->int_reported) {
		pthread_mutex_unlock(&CHAN->int_mutex);
		return;
	}
	pthread_mutex_unlock(&CHAN->int_mutex);

	// check if any unit reported interrupt
	for (int unit_n=0 ; unit_n<CMEM_MAX_DEVICES ; unit_n++) {
		pthread_mutex_lock(&CHAN->int_mutex);
		if ((CHAN->int_unit[unit_n] != CMEM_INT_NONE) && !CHAN->int_mask) {
			chan->int_reported = unit_n;
			pthread_mutex_unlock(&CHAN->int_mutex);
			LOG(L_CMEM, 3, "CMEM (ch:%i) reporting interrupt %i", chan->proto.num, chan->proto.num + 12);
			int_set(chan->proto.num + 12);
			break;
		} else {
			pthread_mutex_unlock(&CHAN->int_mutex);
		}
	}
}

// -----------------------------------------------------------------------
void cmem_int(struct cmem_chan_t *chan, int unit_n, int interrupt)
{
	LOG(L_CMEM, 1, "CMEM (ch:%i) interrupt %i, unit: %i", chan->proto.num, interrupt, unit_n);
	pthread_mutex_lock(&CHAN->int_mutex);
	if (interrupt < CHAN->int_unit[unit_n]) {
		CHAN->int_unit[unit_n] = interrupt;
	}
	pthread_mutex_unlock(&CHAN->int_mutex);

	cmem_int_report(CHAN);
}

// -----------------------------------------------------------------------
int cmem_cmd_intspec(struct chan *chan, uint16_t *r_arg)
{
	LOG(L_CMEM, 1, "CMEM (ch:%i) command: intspec", chan->num);

	pthread_mutex_lock(&CHAN->int_mutex);
	if (CHAN->int_reported != -1) {
		*r_arg = (CHAN->was_en << 15) | (CHAN->int_unit[CHAN->int_reported] << 8) | (CHAN->int_reported << 5);
		// mark interrupt as served
		CHAN->int_unit[CHAN->int_reported] = CMEM_INT_NONE;
		// nothing new reported
		CHAN->int_reported = -1;
	} else {
		*r_arg = 0;
	}
	pthread_mutex_unlock(&CHAN->int_mutex);

	// this is where we always unlock after transmission command
	// (TODO: but may it be unlocked earlier by the unit?)
	pthread_mutex_unlock(&CHAN->transmit_mutex);

	// report another interrupt if it's there
	cmem_int_report(CHAN);

	return IO_OK;
}

// -----------------------------------------------------------------------
int cmem_chan_cmd(struct chan *chan, int dir, int cmd, int u_num, uint16_t *r_arg)
{
	// any command makes channel take of the interrupt mask
	pthread_mutex_lock(&CHAN->int_mutex);
	CHAN->int_mask = 0;
	pthread_mutex_unlock(&CHAN->int_mutex);

	if (dir == IO_OU) {
		switch (cmd) {
		case CHAN_CMD_EXISTS:
			LOG(L_CMEM, 1, "CMEM %i (%s): command: check chan exists", chan->num, chan->name);
			break;
		case CHAN_CMD_INTSPEC:
			return cmem_cmd_intspec(chan, r_arg);
		case CHAN_CMD_STATUS:
			*r_arg = CHAN->untransmitted;
			LOG(L_CMEM, 1, "CMEM %i (%s) command: get status", chan->num, chan->name);
			break;
		case CHAN_CMD_ALLOC:
			// all units always working with CPU 0
			*r_arg = 0;
			LOG(L_CMEM, 1, "CMEM %i:%i (%s): command: get allocation -> %i", chan->num, u_num, chan->name, *r_arg);
			break;
		default:
			LOG(L_CMEM, 1, "CMEM %i:%i (%s): unknow command", chan->num, u_num, chan->name);
			// shouldn't happen, but as channel always reports OK...
			break;
		}
	} else {
		switch (cmd) {
		case CHAN_CMD_EXISTS:
			LOG(L_CMEM, 1, "CMEM %i (%s): command: check chan exists", chan->num, chan->name);
			break;
		case CHAN_CMD_MASK_PN:
			CHAN->int_mask = 1;
			LOG(L_CMEM, 1, "CMEM %i (%s): command: mask CPU", chan->num, chan->name);
			break;
		case CHAN_CMD_MASK_NPN:
			LOG(L_CMEM, 1, "CMEM %i (%s): command: mask ~CPU -> ignored", chan->num, chan->name);
			// ignore 2nd CPU
			break;
		case CHAN_CMD_ASSIGN:
			LOG(L_CMEM, 1, "CMEM %i (%s:%s): command: assign CPU -> ignored", chan->num, u_num, chan->name);
			// always for CPU 0
			break;
		default:
			LOG(L_CMEM, 1, "CMEM %i:%i (%s): unknow command", chan->num, u_num, chan->name);
			// shouldn't happen, but as channel always reports OK...
			break;
		}
	}
	return IO_OK;
}

// -----------------------------------------------------------------------
int cmem_cmd(struct chan *chan, int dir, uint16_t n_arg, uint16_t *r_arg)
{
	int cmd		= (n_arg & 0b1111110000000000) >> 10;
	int u_num	= (n_arg & 0b0000000011100000) >> 5;

	if ((cmd & 0b111000) == 0) { // command for channel
		return cmem_chan_cmd(chan, dir, cmd, u_num, r_arg);
	} else { // command for unit
	 	// transmission command
	 	if ((cmd & 0b11100) == 0b11000) {
			// only one transmission command at a time
			int res = pthread_mutex_trylock(&CHAN->transmit_mutex);
			if (res) {
				return IO_EN;
			}
		}
		if (CHAN->unit[u_num]) {
			return CHAN->unit[u_num]->cmd(CHAN->unit[u_num], dir, cmd, r_arg);
		} else {
			return IO_NO;
		}
	}
}

// vim: tabstop=4 shiftwidth=4 autoindent
