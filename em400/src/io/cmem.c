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

#include "cpu/interrupts.h"
#include "io/io.h"
#include "io/chan.h"
#include "io/cmem.h"
#include "io/cmem_m9425.h"

#include "cfg.h"
#include "errors.h"

#include "debugger/log.h"

#define CHAN ((struct cmem_chan_t *)(chan))

// unit prototypes
struct cmem_unit_proto_t cmem_unit_proto[] = {
	{
		"mera9425",
		cmem_m9425_create,
		cmem_m9425_shutdown,
		cmem_m9425_reset
	},
	{
		NULL,
		NULL,
		NULL,
		NULL
	}
};

// -----------------------------------------------------------------------
struct chan_proto_t * cmem_create(struct cfg_unit_t *units)
{
	struct cmem_chan_t *chan = calloc(1, sizeof(struct cmem_chan_t));
	pthread_mutex_init(&chan->transmit_mutex, NULL);
	pthread_mutex_init(&chan->int_mutex, NULL);

	return (struct chan_proto_t *) chan;
}

// -----------------------------------------------------------------------
void cmem_shutdown(struct chan_proto_t *chan)
{
	free(chan);
}

// -----------------------------------------------------------------------
void cmem_reset(struct chan_proto_t *chan)
{
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
		int is_masked = CHAN->int_mask & (1 << unit_n);
		if ((CHAN->int_unit[unit_n] != CMEM_INT_NONE) && !is_masked) {
			chan->int_reported = unit_n;
			pthread_mutex_unlock(&CHAN->int_mutex);
			LOG(D_IO, 20, "CMEM (ch:%i) reporting interrupt %i", chan->proto.num, chan->proto.num + 12);
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
	LOG(D_IO, 1, "CMEM (ch:%i) interrupt %i, unit: %i", chan->proto.num, interrupt, unit_n);
	pthread_mutex_lock(&CHAN->int_mutex);
	if (interrupt < CHAN->int_unit[unit_n]) {
		CHAN->int_unit[unit_n] = interrupt;
	}
	pthread_mutex_unlock(&CHAN->int_mutex);

	cmem_int_report(CHAN);
}

// -----------------------------------------------------------------------
int cmem_cmd_intspec(struct chan_proto_t *chan, uint16_t *r_arg)
{
	LOG(D_IO, 1, "CMEM (ch:%i) command: intspec", chan->num);

	pthread_mutex_lock(&CHAN->int_mutex);
	if (CHAN->int_reported != -1) {
		*r_arg = (CHAN->was_en << 15) || (CHAN->int_unit[CHAN->int_reported] << 8) || (CHAN->int_reported << 5);
		// mark interrupt as served
		CHAN->int_unit[CHAN->int_reported] = CMEM_INT_NONE;
		// nothing new reported
		CHAN->int_reported = -1;
	} else {
		*r_arg = 0;
	}
	pthread_mutex_unlock(&CHAN->int_mutex);

	// report another interrupt if it's there
	cmem_int_report(CHAN);

	return IO_OK;
}

// -----------------------------------------------------------------------
int cmem_chan_cmd(struct chan_proto_t *chan, int dir, int cmd, int u_num, uint16_t *r_arg)
{
	if (dir == IO_OU) {
		switch (cmd) {
		case CHAN_CMD_EXISTS:
			LOG(D_IO, 1, "CMEM %i (%s): CHAN_CMD_EXISTS", chan->num, chan->name);
			break;
		case CHAN_CMD_INTSPEC:
			return cmem_cmd_intspec(chan, r_arg);
		case CHAN_CMD_STATUS:
			//*r_arg = chan->untransmitted;
			LOG(D_IO, 1, "CMEM %i (%s) CHAN_CMD_STATUS", chan->num, chan->name);
			break;
		case CHAN_CMD_ALLOC:
			// all units always working with CPU 0
			*r_arg = 0;
			LOG(D_IO, 1, "CMEM %i:%i (%s): CHAN_CMD_ALLOC -> %i", chan->num, u_num, chan->name, *r_arg);
			break;
		default:
			// shouldn't happen, but as channel always reports OK...
			break;
		}
	} else {
		switch (cmd) {
		case CHAN_CMD_EXISTS:
			LOG(D_IO, 1, "CMEM %i (%s): CHAN_CMD_EXISTS", chan->num, chan->name);
			break;
		case CHAN_CMD_MASK_PN:
			//chan->int_mask = 1;
			LOG(D_IO, 1, "CMEM %i (%s): CHAN_CMD_MASK_PN", chan->num, chan->name);
			break;
		case CHAN_CMD_MASK_NPN:
			LOG(D_IO, 1, "CMEM %i (%s): CHAN_CMD_MASK_NPN -> ignored", chan->num, chan->name);
			// ignore 2nd CPU
			break;
		case CHAN_CMD_ASSIGN:
			LOG(D_IO, 1, "CMEM %i (%s:%s): CHAN_CMD_ASSIGN -> ignored", chan->num, u_num, chan->name);
			// always for CPU 0
			break;
		default:
			LOG(D_IO, 1, "CMEM %i:%i (%s): unknow command", chan->num, u_num, chan->name);
			// shouldn't happen, but as channel always reports OK...
			break;
		}
	}
	return IO_OK;
}

// -----------------------------------------------------------------------
int cmem_cmd(struct chan_proto_t *chan, int dir, uint16_t n_arg, uint16_t *r_arg)
{
	int u_num = (n_arg & 0b0000000011100000) >> 5;
	int cmd = (n_arg & 0b1111110000000000) >> 10;

	// command for channel
	if ((cmd & 0b111000) == 0) {
		return cmem_chan_cmd(chan, dir, cmd, u_num, r_arg);
	// command for unit
	} else {
	 	// transmission command
	 	if ((cmd & 0b11100) == 0b11000) {
			int res = pthread_mutex_trylock(&CHAN->transmit_mutex);
			if (res) {
				return IO_EN;
			}
		}
		return CHAN->unit[u_num]->cmd(CHAN->unit[u_num], dir, cmd, r_arg);
	}
}

// vim: tabstop=4 shiftwidth=4 autoindent
