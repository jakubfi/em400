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
#include "io/cchar_flop8.h"

#include "log.h"
#include "cfg.h"

// unit prototypes
struct cchar_unit_proto_t cchar_unit_proto[] = {
	{
		"terminal",
		cchar_term_create,
		cchar_term_shutdown,
		cchar_term_reset,
		cchar_term_cmd
	},
	{
		"floppy8",
		cchar_flop8_create,
		cchar_flop8_shutdown,
		cchar_flop8_reset,
		cchar_flop8_cmd
	},
	{ NULL, NULL, NULL, NULL, NULL }
};

// -----------------------------------------------------------------------
static struct cchar_unit_proto_t * cchar_unit_proto_get(struct cchar_unit_proto_t *proto, const char *name)
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
void * cchar_create(int ch_num, em400_cfg *cfg)
{
	struct cchar_chan_t *chan = (struct cchar_chan_t *) calloc(1, sizeof(struct cchar_chan_t));

	chan->num = ch_num;
	for (int dev_num=0 ; dev_num<CCHAR_MAX_DEVICES ; dev_num++) {
		// find unit prototype
		const char *unit_name = cfg_fgetstr(cfg, "dev%i.%i:type", ch_num, dev_num);
		if (!unit_name) continue;
		struct cchar_unit_proto_t *proto = cchar_unit_proto_get(cchar_unit_proto, unit_name);
		if (!proto) {
			LOGERR("Unknown device type or device incompatibile with channel: %s.", unit_name);
			free(chan);
			return NULL;
		}

		// create unit based on prototype
		struct cchar_unit_proto_t *unit = proto->create(cfg, ch_num, dev_num);
		if (!unit) {
			LOGERR("Failed to create unit: %s.", unit_name);
			free(chan);
			return NULL;
		} else {
			LOG(L_CCHR, "Connected device %i: %s", dev_num, proto->name);
		}

		// fill in functions
		unit->name = proto->name;
		unit->create = proto->create;
		unit->shutdown = proto->shutdown;
		unit->reset = proto->reset;
		unit->cmd = proto->cmd;

		// remember the channel unit is connected to
		unit->chan = chan;
		unit->num = dev_num;

		chan->unit[dev_num] = unit;

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
	if (!chan) return;

	struct cchar_chan_t *ch = (struct cchar_chan_t *) chan;

	for (int i=0 ; i<CCHAR_MAX_DEVICES ; i++) {
		struct cchar_unit_proto_t *u = ch->unit[i];
		if (u) {
			u->shutdown(u);
		}
	}
	free(chan);
}

// -----------------------------------------------------------------------
void cchar_reset(void *chan)
{
	if (!chan) return;

	struct cchar_chan_t *ch = (struct cchar_chan_t *) chan;

	for (int i=0 ; i<CCHAR_MAX_DEVICES ; i++) {
		struct cchar_unit_proto_t *u = ch->unit[i];
		if (u) {
			u->reset(u);
		}
	}
	pthread_mutex_lock(&ch->int_mutex);
	ch->int_reported = -1;
	pthread_mutex_unlock(&ch->int_mutex);
}

// -----------------------------------------------------------------------
static void cchar_int_report(struct cchar_chan_t *chan)
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
			LOG(L_CCHR, "CCHAR (ch:%i) reporting interrupt %i", chan->num, chan->num + 12);
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
	LOG(L_CCHR, "CCHAR (ch:%i) interrupt %i, unit: %i", chan->num, interrupt, unit_n);

	pthread_mutex_lock(&chan->int_mutex);
	chan->int_unit[unit_n] = interrupt;
	pthread_mutex_unlock(&chan->int_mutex);

	cchar_int_report(chan);
}

// -----------------------------------------------------------------------
static int cchar_cmd_intspec(struct cchar_chan_t *chan, uint16_t *r_arg)
{
	LOG(L_CCHR, "CCHAR (ch:%i) command: intspec", chan->num);

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
static int cchar_chan_cmd(struct cchar_chan_t *chan, int dir, int cmd, int u_num, uint16_t *r_arg)
{
	if (dir == IO_OU) {
		switch (cmd) {
		case CHAN_CMD_EXISTS:
			LOG(L_CCHR, "CCHAR %i: command: check chan exists", chan->num);
			break;
		case CHAN_CMD_MASK_PN:
			chan->int_mask = 1;
			LOG(L_CCHR, "CCHAR %i: command: mask CPU", chan->num);
			break;
		case CHAN_CMD_MASK_NPN:
			LOG(L_CCHR, "CCHAR %i: command: mask ~CPU (ignored)", chan->num);
			break;
		case CHAN_CMD_ASSIGN:
			// nothing to assign, always assigned to CPU 0
			LOG(L_CCHR, "CCHAR %i:%i: command: assign CPU (ignored)", chan->num, u_num);
			break;
		default:
			LOG(L_CCHR, "CCHAR %i:%i: unknow command", chan->num, u_num);
			// shouldn't happen, but as channel always reports OK...
			break;
		}
	} else {
		switch (cmd) {
		case CHAN_CMD_EXISTS:
		case CHAN_CMD_STATUS:
			LOG(L_CCHR, "CCHAR %i: command: check chan exists", chan->num);
			break;
		case CHAN_CMD_INTSPEC:
			return cchar_cmd_intspec(chan, r_arg);
		case CHAN_CMD_ALLOC:
			// all units always working with CPU 0
			*r_arg = 0;
			LOG(L_CCHR, "CCHAR %i:%i: command: get allocation -> %i", chan->num, u_num, *r_arg);
			break;
		default:
			LOG(L_CCHR, "CCHAR %i:%i: unknow command", chan->num, u_num);
			// shouldn't happen, but as channel always reports OK...
			break;
		}
	}

	return IO_OK;
}

// -----------------------------------------------------------------------
int cchar_cmd(void *ch, int dir, uint16_t n_arg, uint16_t *r_arg)
{
	const unsigned cmd = (n_arg & 0b1111110000000000) >> 10;
	const unsigned u_num = (n_arg & 0b0000000011100000) >> 5;
	const unsigned is_chan_cmd = (cmd & 0b111000) == 0;

	struct cchar_chan_t *chan = (struct cchar_chan_t *) ch;

	if (is_chan_cmd) {
		return cchar_chan_cmd(chan, dir, cmd, u_num, r_arg);
	} else {
		struct cchar_unit_proto_t *u = (struct cchar_unit_proto_t *) chan->unit[u_num];
		if (u) {
			return u->cmd(u, dir, cmd, r_arg);
		} else {
			return IO_NO;
		}
	}
}

// -----------------------------------------------------------------------
struct chan_drv cchar_chan_driver = {
	.name = "char",
	.create = cchar_create,
	.shutdown = cchar_shutdown,
	.reset = cchar_reset,
	.cmd = cchar_cmd
};

// vim: tabstop=4 shiftwidth=4 autoindent
