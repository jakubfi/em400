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

#define _XOPEN_SOURCE 500
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>
#include <strings.h>
#include <uv.h>
#include <pthread.h>

#include "io/io.h"
#include "io/chan.h"
#include "io/cchar/cchar.h"
#include "io/cchar/term.h"
#include "io/cchar/uzdat.h"
#include "io/cchar/flop8.h"

#include "log.h"
#include "cfg.h"

uv_loop_t *ioloop;

#define NO_INTERRUPT_REPORTED -1

// unit prototypes
cchar_unit_proto_t cchar_unit_proto[] = {
	{
		"terminalold",
		cchar_term_create,
		cchar_term_shutdown,
		NULL,
		cchar_term_reset,
		cchar_term_cmd,
		cchar_term_intspec,
		cchar_term_has_interrupt,
	},
	{
		"terminal",
		uzdat_create,
		uzdat_shutdown,
		uzdat_free,
		uzdat_reset,
		uzdat_cmd,
		uzdat_intspec,
		uzdat_has_interrupt,
	},
	{
		"floppy8",
		cchar_flop8_create,
		cchar_flop8_shutdown,
		NULL,
		cchar_flop8_reset,
		cchar_flop8_cmd,
		cchar_flop8_intspec,
		cchar_flop8_has_interrupt,
	},
	{ NULL, NULL, NULL, NULL, NULL, NULL, NULL }
};

// -----------------------------------------------------------------------
static cchar_unit_proto_t * cchar_unit_proto_get(cchar_unit_proto_t *proto, const char *name)
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
static void cchar_ioloop_teardown(cchar_chan_t *chan)
{
	uv_close((uv_handle_t *) &chan->async_quit, NULL);
}

// -----------------------------------------------------------------------
static void cchar_on_async_quit(uv_async_t *handle)
{
	LOG(L_CCHR, "QUIT received");

	uv_stop(ioloop);
}

// -----------------------------------------------------------------------
static void cchar_ioloop_setup(cchar_chan_t *chan)
{
	ioloop = uv_default_loop();

	uv_async_init(ioloop, &chan->async_quit, cchar_on_async_quit);
	uv_handle_set_data((uv_handle_t*) &chan->async_quit, chan);
}

// -----------------------------------------------------------------------
static void * cchar_ioloop(void *ptr)
{
	LOG(L_CCHR, "Starting UV loop");
	uv_run(ioloop, UV_RUN_DEFAULT);
	LOG(L_CCHR, "Exited UV loop");

	pthread_exit(NULL);
}

// -----------------------------------------------------------------------
void * cchar_create(int ch_num, em400_cfg *cfg)
{
	cchar_chan_t *chan = (cchar_chan_t *) calloc(1, sizeof(cchar_chan_t));

	cchar_ioloop_setup(chan);

	chan->num = ch_num;
	for (int dev_num=0 ; dev_num<CCHAR_MAX_DEVICES ; dev_num++) {
		// find unit prototype
		const char *unit_name = cfg_fgetstr(cfg, "dev%i.%i:type", ch_num, dev_num);
		if (!unit_name) continue;
		cchar_unit_proto_t *proto = cchar_unit_proto_get(cchar_unit_proto, unit_name);
		if (!proto) {
			LOGERR("Unknown device type or device incompatibile with channel: %s.", unit_name);
			free(chan);
			return NULL;
		}

		// create unit based on prototype
		cchar_unit_proto_t *unit = proto->create(cfg, ch_num, dev_num);
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
		unit->free = proto->free;
		unit->reset = proto->reset;
		unit->cmd = proto->cmd;
		unit->intspec = proto->intspec;
		unit->has_interrupt = proto->has_interrupt;

		// remember the channel unit is connected to
		unit->chan = chan;
		unit->num = dev_num;

		chan->unit[dev_num] = unit;
	}

	// clear unit interrupts
	pthread_mutex_lock(&chan->int_mutex);
	chan->interrupting_device = NO_INTERRUPT_REPORTED;
	pthread_mutex_unlock(&chan->int_mutex);

	if (pthread_create(&chan->ioloop_thread, NULL, cchar_ioloop, chan)) {
		LOGERR("Failed to spawn main I/O tester thread.");
		goto fail;
	}
	pthread_setname_np(chan->ioloop_thread, "ioloop");

	return (void *) chan;
fail:
	return NULL;
}

// -----------------------------------------------------------------------
void cchar_shutdown(void *chan)
{
	if (!chan) return;

	cchar_chan_t *ch = (cchar_chan_t *) chan;

	// stop ioloop and its thread
	uv_async_send(&ch->async_quit);
	pthread_join(ch->ioloop_thread, NULL);

	// shutdown (stop) all connected controllers
	for (int i=0 ; i<CCHAR_MAX_DEVICES ; i++) {
		cchar_unit_proto_t *u = ch->unit[i];
		if (u && u->shutdown) {
			u->shutdown(u);
		}
	}

	// clean ioloop resources
	cchar_ioloop_teardown(chan);
	// give libuv chance to cleanup handles
	uv_run(ioloop, UV_RUN_DEFAULT);
	int res = uv_loop_close(ioloop);
	if (res < 0) {
		LOG(L_CCHR, "I/O loop failed to close nicely: %s", uv_strerror(res));
	}

	// free all connected controllers' resources
	for (int i=0 ; i<CCHAR_MAX_DEVICES ; i++) {
		cchar_unit_proto_t *u = ch->unit[i];
		if (u && u->free) {
			u->free(u);
		}
	}

	// free channel resources
	free(chan);
}

// -----------------------------------------------------------------------
void cchar_reset(void *chan)
{
	if (!chan) return;

	cchar_chan_t *ch = (cchar_chan_t *) chan;

	for (int i=0 ; i<CCHAR_MAX_DEVICES ; i++) {
		cchar_unit_proto_t *u = ch->unit[i];
		if (u) {
			u->reset(u);
		}
	}

	pthread_mutex_lock(&ch->int_mutex);
	ch->interrupting_device = NO_INTERRUPT_REPORTED;
	pthread_mutex_unlock(&ch->int_mutex);
}

// -----------------------------------------------------------------------
void cchar_int_trigger(cchar_chan_t *chan)
{
	pthread_mutex_lock(&chan->int_mutex);
	if (chan->interrupting_device != NO_INTERRUPT_REPORTED) {
		// interrupt reported to the CPU but not yet served, nothing more to do
		LOG(L_CCHR, "CCHAR (ch:%i) not reporting interrupt. Reported by unit: %i has yet to be served", chan->num, chan->interrupting_device);
	} else {
		// check if any unit reported interrupt
		for (int unit_n=0 ; unit_n<CCHAR_MAX_DEVICES ; unit_n++) {
			cchar_unit_proto_t *unit = chan->unit[unit_n];
			if (unit && unit->has_interrupt(unit)) {
				LOG(L_CCHR, "CCHAR (ch:%i) reporting interrupt from unit %i", chan->num, unit_n);
				chan->interrupting_device = unit_n;
				io_int_set(chan->num);
				break;
			}
		}
		LOG(L_CCHR, "CCHAR (ch:%i) No more interrupt lines active", chan->num);
	}
	pthread_mutex_unlock(&chan->int_mutex);
}

// -----------------------------------------------------------------------
void cchar_int_cancel(cchar_chan_t *chan, int unit_n)
{
	LOG(L_CCHR, "CCHAR (ch:%i) unit: %i cancel interrupt", chan->num, unit_n);

	pthread_mutex_lock(&chan->int_mutex);
	if (chan->interrupting_device == unit_n) {
		chan->interrupting_device = NO_INTERRUPT_REPORTED;
	}
	pthread_mutex_unlock(&chan->int_mutex);

	cchar_int_trigger(chan);
}

// -----------------------------------------------------------------------
static int cchar_cmd_intspec(cchar_chan_t *chan, uint16_t *r_arg)
{
	pthread_mutex_lock(&chan->int_mutex);
	if (chan->interrupting_device != NO_INTERRUPT_REPORTED) {
		cchar_unit_proto_t *unit = chan->unit[chan->interrupting_device];
		int intspec = unit->intspec(unit);
		LOG(L_CCHR, "CCHAR (ch:%i) device %i interrupt specification: %i", chan->num, chan->interrupting_device, intspec);
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
static int cchar_chan_cmd(cchar_chan_t *chan, int dir, int cmd, int u_num, uint16_t *r_arg)
{
	if (dir == IO_OU) {
		switch (cmd) {
		case CHAN_CMD_EXISTS:
			LOG(L_CCHR, "CCHAR %i: command: check chan exists", chan->num);
			break;
		case CHAN_CMD_MASK_PN:
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
			LOG(L_CCHR, "CCHAR %i:%i: unknown command", chan->num, u_num);
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

	cchar_chan_t *chan = (cchar_chan_t *) ch;

	if (is_chan_cmd) {
		return cchar_chan_cmd(chan, dir, cmd, u_num, r_arg);
	} else {
		cchar_unit_proto_t *u = (cchar_unit_proto_t *) chan->unit[u_num];
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
