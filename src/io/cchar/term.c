//  Copyright (c) 2013-2020, 2022 Jakub Filipowicz <jakubf@gmail.com>
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
#include <strings.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdatomic.h>

#include "em400.h"
#include "io/defs.h"
#include "io/cchar/term.h"
#include "io/dev/fdbridge.h"

#include "log.h"
#include "cfg.h"

static int fdb_callback(void *user_ctx, int condition);

// -----------------------------------------------------------------------
cchar_unit_proto_t * cchar_term_create(em400_cfg *cfg, int ch_num, int dev_num)
{
	cchar_unit_term_t *term = (cchar_unit_term_t *) calloc(1, sizeof(cchar_unit_term_t));
	if (!term) {
		LOGERR("Failed to allocate memory for terminal: %i.%i", ch_num, dev_num);
		return NULL;
	}

	const char *transport = cfg_fgetstr(cfg, "dev%i.%i:transport", ch_num, dev_num);
	const int speed = cfg_fgetint(cfg, "dev%i.%i:speed", ch_num, dev_num);
	const char *device = cfg_fgetstr(cfg, "dev%i.%i:device", ch_num, dev_num);
	const int port = cfg_fgetint(cfg, "dev%i.%i:port", ch_num, dev_num);

	LOG(L_TERM, "Creating terminal: transport: %s, speed: %i, device: %s, port: %i", transport, speed, device, port);

	if (!strcasecmp(transport, "tcp")) {
		if (!port) {
			LOGERR("TCP terminal needs port to be set.");
			goto fail;
		}
		term->fdbridge = fdb_open_tcp(port);
		if (speed > 0) {
			fdb_set_speed(term->fdbridge, speed);
		}
	} else if (!strcasecmp(transport, "serial")) {
		if (!device || (speed < 0)) {
			LOGERR("Serial terminal needs both 'device' and 'speed' configuration.");
			goto fail;
		}
		term->fdbridge = fdb_open_serial(device, speed);
	} else if (!strcasecmp(transport, "console")) {
		if (em400_console == CONSOLE_DEBUGGER) {
			LOGERR("Failed to initialize console terminal; console is being used by the debugger.");
			goto fail;
		} else if (em400_console == CONSOLE_TERMINAL) {
			LOGERR("Failed to initialize console terminal; console is being used by another terminal.");
			goto fail;
		}
		em400_console = CONSOLE_TERMINAL;
		term->fdbridge = fdb_open_stdin();
	} else {
		LOGERR("Unknown terminal transport: %s.", transport);
		goto fail;
	}

	if (!term->fdbridge) {
		LOGERR("Failed to initialize terminal.");
		goto fail;
	}

	fdb_set_callback(term->fdbridge, fdb_callback, term);

	return (cchar_unit_proto_t *) term;

fail:
	cchar_term_shutdown((cchar_unit_proto_t *) term);
	return NULL;
}

// -----------------------------------------------------------------------
void cchar_term_shutdown(cchar_unit_proto_t *unit)
{
	cchar_unit_term_t *term = (cchar_unit_term_t *) unit;
	if (!term) return;

	if (term->fdbridge) fdb_close(term->fdbridge);
	free(term);
}

// -----------------------------------------------------------------------
void cchar_term_reset(cchar_unit_proto_t *unit)
{
	LOG(L_TERM, "Command: reset");
	cchar_unit_term_t *term = (cchar_unit_term_t *) unit;
	fdb_reset(term->fdbridge);
}

// -----------------------------------------------------------------------
static int fdb_callback(void *ctx, int condition)
{
	cchar_unit_term_t *term = (cchar_unit_term_t*) ctx;
	int interrupt = CCHAR_TERM_INT_OUTDATED;

	switch (condition) {
		case FDB_READY:
			LOG(L_TERM, "Callback: line data ready");
			interrupt = CCHAR_TERM_INT_READY;
			break;
		case FDB_LOST:
			LOG(L_TERM, "Callback: data lost, transmission too slow");
			interrupt = CCHAR_TERM_INT_TOO_SLOW;
			break;
		default:
			break;
	}

	if (interrupt != CCHAR_TERM_INT_OUTDATED) {
		atomic_store_explicit(&term->spec, interrupt, memory_order_release);
		cchar_int_trigger(term->proto.chan);
	}

	return 0;
}

// -----------------------------------------------------------------------
int cchar_term_intspec(cchar_unit_proto_t *unit)
{
	cchar_unit_term_t *term = (cchar_unit_term_t*) unit;
	int spec = atomic_load_explicit(&term->spec, memory_order_acquire);
	atomic_store_explicit(&term->spec, CCHAR_TERM_INT_OUTDATED, memory_order_release);

	return spec;
}

// -----------------------------------------------------------------------
bool cchar_term_has_interrupt(cchar_unit_proto_t *unit)
{
	cchar_unit_term_t *term = (cchar_unit_term_t*) unit;
	return atomic_load_explicit(&term->spec, memory_order_acquire) ? true : false;
}

// -----------------------------------------------------------------------
static int cchar_term_read(cchar_unit_term_t *term, uint16_t *r_arg)
{
	int data = fdb_read(term->fdbridge);

	if (data < 0) {
		LOG(L_TERM, "Receive from terminal: empty read");
		return IO_EN;
	}

	if ((data >= 32) && (data < 127)) {
		LOG(L_TERM, "Receive from terminal: %i (%c)", data, data);
	} else {
		LOG(L_TERM, "Receive from terminal: %i (#%02x)", data, data);
	}

	*r_arg = data;
	return IO_OK;
}

// -----------------------------------------------------------------------
static int cchar_term_write(cchar_unit_term_t *term, uint16_t *r_arg)
{
	int res = IO_OK;
	char data = *r_arg & 0xff;

	if (fdb_write(term->fdbridge, data) < 0) {
		res = IO_EN;
	}

	if ((data >= 32) && (data < 127)) {
		LOG(L_TERM, "Send to terminal: %i (%c)%s", data, data, res == IO_EN ? " failed (transmitter busy)" : "");
	} else {
		LOG(L_TERM, "Send to terminal: %i (#%02x)%s", data, data, res == IO_EN ? " failed (transmitter busy)" : "");
	}

	return res;
}

// -----------------------------------------------------------------------
static int cchar_term_disconnect(cchar_unit_term_t *term)
{
	LOG(L_TERM, "Command: disconnect");
	fdb_await_read(term->fdbridge);
	return IO_OK;
}

// -----------------------------------------------------------------------
int cchar_term_cmd(cchar_unit_proto_t *unit, int dir, int cmd, uint16_t *r_arg)
{
	cchar_unit_term_t *term = (cchar_unit_term_t *) unit;

	if (dir == IO_IN) {
		switch (cmd) {
			case CCHAR_TERM_CMD_SPU:
				LOG(L_TERM, "Command: SPU");
				return IO_OK;
			case CCHAR_TERM_CMD_READ:
				return cchar_term_read(term, r_arg);
			default:
				LOG(L_TERM, "Unknown IN command: %i", cmd);
				return IO_EN;
		}
	} else {
		switch (cmd) {
			case CCHAR_TERM_CMD_RESET:
				cchar_term_reset(unit);
				return IO_OK;
			case CCHAR_TERM_CMD_DISCONNECT:
				return cchar_term_disconnect(term);
			case CCHAR_TERM_CMD_WRITE:
				return cchar_term_write(term, r_arg);
			default:
				LOG(L_TERM, "Unknown OU command: %i", cmd);
				return IO_EN;
		}
	}
}

// vim: tabstop=4 shiftwidth=4 autoindent
