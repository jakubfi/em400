//  Copyright (c) 2012-2025 Jakub Filipowicz <jakubf@gmail.com>
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
#include <unistd.h>
#include <strings.h>

#include "io/io.h"
#include "io/chan.h"
#include "io/cchar/cchar.h"
#include "io/cchar/uzdat.h"
#include "io/cchar/uzfx.h"
#include "io/dev/terminal.h"

#include "log.h"

#define NO_INTERRUPT_REPORTED -1
#define CCHAR_MAX_DEVICES 8
#define CCHAR_INT_NONE 9999 // no interrupt (em400 sentinel)

struct chan_char {
	chan_t base;

	pthread_mutex_t int_mutex;
	int int_mask;
	int interrupting_device;
	int was_en;
	int untransmitted;

	cchar_unit_t *unit[CCHAR_MAX_DEVICES];
};

void cchar_shutdown(chan_t *chan);
void cchar_reset(chan_t *chan);
int cchar_cmd(chan_t *ch, int dir, uint16_t n_arg, uint16_t *r_arg);

// -----------------------------------------------------------------------
int cchar_connect_dev(chan_t *chan, int devnum, em400_dev_t *dev)
{
	chan_char_t *cchar = (chan_char_t *) chan;
	cchar_unit_t *unit = NULL;

	switch (dev->type) {
		case EM400_DEV_TERMINAL:
			unit = uzdat_create(devnum, dev);
			break;
		case EM400_DEV_SP45DE:
			unit = uzfx_create(devnum, dev);
			break;
		default:
			return LOGERR("Device type unknown or incompatibile with character channel");
	}

	if (!unit) {
		return LOGERR("Error creating device controller");
	}

	unit->chan = cchar;
	cchar->unit[devnum] = unit;

	return E_OK;
}

// -----------------------------------------------------------------------
chan_t * cchar_create(int ch_num)
{
	chan_char_t *chan = calloc(1, sizeof(chan_char_t));
	if (!chan) {
		LOGERR("Memory allocation error");
		return NULL;
	}

	chan->base.num = ch_num;
	chan->base.type = EM400_CHANNEL_CHAR;
	chan->base.cmd = cchar_cmd;
	chan->base.reset = cchar_reset;
	chan->base.shutdown = cchar_shutdown;
	chan->base.connect_dev = cchar_connect_dev;

	chan->interrupting_device = NO_INTERRUPT_REPORTED;

	return (chan_t *) chan;
}

// -----------------------------------------------------------------------
void cchar_shutdown(chan_t *chan)
{
	if (!chan) return;
	chan_char_t *ch = (chan_char_t *) chan;

	LOG(L_CCHR, "Shutting down CHAR channel %i", ch->base.num);

	// stop all connected controllers
	for (int i=0 ; i<CCHAR_MAX_DEVICES ; i++) {
		cchar_unit_t *u = ch->unit[i];
		if (u && u->shutdown) {
			u->shutdown(u);
		}
	}

	LOG(L_CCHR, "All units shut down");
	free(chan);
}

// -----------------------------------------------------------------------
void cchar_reset(chan_t *chan)
{
	if (!chan) return;

	chan_char_t *ch = (chan_char_t *) chan;

	for (int i=0 ; i<CCHAR_MAX_DEVICES ; i++) {
		cchar_unit_t *u = ch->unit[i];
		if (u) {
			u->reset(u);
		}
	}

	pthread_mutex_lock(&ch->int_mutex);
	ch->interrupting_device = NO_INTERRUPT_REPORTED;
	pthread_mutex_unlock(&ch->int_mutex);
}

// -----------------------------------------------------------------------
void cchar_int_trigger(chan_char_t *chan)
{
	pthread_mutex_lock(&chan->int_mutex);
	if (chan->interrupting_device != NO_INTERRUPT_REPORTED) {
		// interrupt reported to the CPU but not yet served, nothing more to do
		LOG(L_CCHR, "CCHAR (ch:%i) not reporting interrupt. Reported by unit: %i has yet to be served", chan->base.num, chan->interrupting_device);
	} else {
		// check if any unit reported interrupt
		for (int unit_n=0 ; unit_n<CCHAR_MAX_DEVICES ; unit_n++) {
			cchar_unit_t *unit = chan->unit[unit_n];
			if (unit && unit->has_interrupt(unit)) {
				LOG(L_CCHR, "CCHAR (ch:%i) reporting interrupt from unit %i", chan->base.num, unit_n);
				chan->interrupting_device = unit_n;
				io_int_set(chan->base.num);
				break;
			}
		}
		LOG(L_CCHR, "CCHAR (ch:%i) No more interrupt lines active", chan->base.num);
	}
	pthread_mutex_unlock(&chan->int_mutex);
}

// -----------------------------------------------------------------------
void cchar_int_cancel(chan_char_t *chan, int unit_n)
{
	LOG(L_CCHR, "CCHAR (ch:%i) unit: %i cancel interrupt", chan->base.num, unit_n);

	pthread_mutex_lock(&chan->int_mutex);
	if (chan->interrupting_device == unit_n) {
		chan->interrupting_device = NO_INTERRUPT_REPORTED;
	}
	pthread_mutex_unlock(&chan->int_mutex);

	cchar_int_trigger(chan);
}

// -----------------------------------------------------------------------
static int cchar_cmd_intspec(chan_char_t *chan, uint16_t *r_arg)
{
	pthread_mutex_lock(&chan->int_mutex);
	if (chan->interrupting_device != NO_INTERRUPT_REPORTED) {
		cchar_unit_t *unit = chan->unit[chan->interrupting_device];
		int intspec = unit->intspec(unit);
		LOG(L_CCHR, "CCHAR (ch:%i) device %i interrupt specification: %i", chan->base.num, chan->interrupting_device, intspec);
		*r_arg = (intspec << 8) | (chan->interrupting_device << 5);
		chan->interrupting_device = NO_INTERRUPT_REPORTED;
	} else {
		*r_arg = 0;
	}
	pthread_mutex_unlock(&chan->int_mutex);

	// try reporting another interrupt
	cchar_int_trigger(chan);

	return IO_OK;
}


// -----------------------------------------------------------------------
static int cchar_chan_cmd(chan_char_t *chan, int dir, int cmd, int u_num, uint16_t *r_arg)
{
	if (dir == IO_OU) {
		switch (cmd) {
		case CHAN_CMD_EXISTS:
			LOG(L_CCHR, "CCHAR %i: command: check chan exists", chan->base.num);
			break;
		case CHAN_CMD_MASK_PN:
			LOG(L_CCHR, "CCHAR %i: command: mask CPU (ignored)", chan->base.num);
			break;
		case CHAN_CMD_MASK_NPN:
			LOG(L_CCHR, "CCHAR %i: command: mask ~CPU (ignored)", chan->base.num);
			break;
		case CHAN_CMD_ASSIGN:
			// nothing to assign, always assigned to CPU 0
			LOG(L_CCHR, "CCHAR %i:%i: command: assign CPU (ignored)", chan->base.num, u_num);
			break;
		default:
			LOG(L_CCHR, "CCHAR %i:%i: unknown command", chan->base.num, u_num);
			// shouldn't happen, but as channel always reports OK...
			break;
		}
	} else {
		switch (cmd) {
		case CHAN_CMD_EXISTS:
		case CHAN_CMD_STATUS:
			LOG(L_CCHR, "CCHAR %i: command: check chan exists", chan->base.num);
			break;
		case CHAN_CMD_INTSPEC:
			return cchar_cmd_intspec(chan, r_arg);
		case CHAN_CMD_ALLOC:
			// all units always working with CPU 0
			*r_arg = 0;
			LOG(L_CCHR, "CCHAR %i:%i: command: get allocation -> %i", chan->base.num, u_num, *r_arg);
			break;
		default:
			LOG(L_CCHR, "CCHAR %i:%i: unknown command", chan->base.num, u_num);
			// shouldn't happen, but as channel always reports OK...
			break;
		}
	}

	return IO_OK;
}

// -----------------------------------------------------------------------
int cchar_cmd(chan_t *ch, int dir, uint16_t n_arg, uint16_t *r_arg)
{
	const unsigned cmd = (n_arg & 0b1111110000000000) >> 10;
	const unsigned u_num = (n_arg & 0b0000000011100000) >> 5;
	const unsigned is_chan_cmd = (cmd & 0b111000) == 0;

	chan_char_t *chan = (chan_char_t *) ch;

	if (is_chan_cmd) {
		return cchar_chan_cmd(chan, dir, cmd, u_num, r_arg);
	} else {
		cchar_unit_t *u = (cchar_unit_t *) chan->unit[u_num];
		if (u) {
			return u->cmd(u, dir, cmd, r_arg);
		} else {
			return IO_NO;
		}
	}
}

// vim: tabstop=4 shiftwidth=4 autoindent
