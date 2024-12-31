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

int fdb_callback(void *user_ctx, int condition);

// -----------------------------------------------------------------------
struct cchar_unit_proto_t * cchar_term_create(em400_cfg *cfg, int ch_num, int dev_num)
{
	struct cchar_unit_term_t *unit = (struct cchar_unit_term_t *) calloc(1, sizeof(struct cchar_unit_term_t));
	if (!unit) {
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
		unit->term = fdb_open_tcp(port);
		if (speed > 0) {
			fdb_set_speed(unit->term, speed);
		}
	} else if (!strcasecmp(transport, "serial")) {
		if (!device || (speed < 0)) {
			LOGERR("Serial terminal needs both 'device' and 'speed' configuration.");
			goto fail;
		}
		unit->term = fdb_open_serial(device, speed);
	} else if (!strcasecmp(transport, "console")) {
		if (em400_console == CONSOLE_DEBUGGER) {
			LOGERR("Failed to initialize console terminal; console is being used by the debugger.");
			goto fail;
		} else if (em400_console == CONSOLE_TERMINAL) {
			LOGERR("Failed to initialize console terminal; console is being used by another terminal.");
			goto fail;
		}
		em400_console = CONSOLE_TERMINAL;
		unit->term = fdb_open_stdin();
	} else {
		LOGERR("Unknown terminal transport: %s.", transport);
		goto fail;
	}

	if (!unit->term) {
		LOGERR("Failed to initialize terminal.");
		goto fail;
	}

	fdb_set_callback(unit->term, fdb_callback, unit);

	return (struct cchar_unit_proto_t *) unit;

fail:
	cchar_term_shutdown((struct cchar_unit_proto_t*) unit);
	return NULL;
}

// -----------------------------------------------------------------------
void cchar_term_shutdown(struct cchar_unit_proto_t *unit)
{
	struct cchar_unit_term_t *u = (struct cchar_unit_term_t *) unit;
	if (!u) return;

	if (u->term) fdb_close(u->term);
	free(u);
}

// -----------------------------------------------------------------------
void cchar_term_reset(struct cchar_unit_proto_t *unit)
{
	LOG(L_TERM, "Command: reset");
	struct cchar_unit_term_t *u = (struct cchar_unit_term_t *) unit;
	fdb_reset(u->term);
}

// -----------------------------------------------------------------------
int fdb_callback(void *ctx, int condition)
{
	struct cchar_unit_proto_t *unit = (struct cchar_unit_proto_t *) ctx;
	struct cchar_unit_term_t *term = (struct cchar_unit_term_t*) ctx;

	switch (condition) {
		case FDB_READY:
			LOG(L_TERM, "Callback: line data ready");
			atomic_store_explicit(&term->spec, CCHAR_TERM_INT_READY, memory_order_release);
			cchar_int_trigger(unit->chan);
			break;
		case FDB_LOST:
			LOG(L_TERM, "Callback: data lost, transmission too slow");
			atomic_store_explicit(&term->spec, CCHAR_TERM_INT_TOO_SLOW, memory_order_release);
			cchar_int_trigger(unit->chan);
		default:
			break;
	}
	return 0;
}

// -----------------------------------------------------------------------
int cchar_term_intspec(struct cchar_unit_proto_t *unit)
{
	struct cchar_unit_term_t *term = (struct cchar_unit_term_t*) unit;
	int spec = atomic_load_explicit(&term->spec, memory_order_acquire);
	atomic_store_explicit(&term->spec, CCHAR_TERM_INT_OUTDATED, memory_order_release);

	return spec;
}

// -----------------------------------------------------------------------
bool cchar_term_has_interrupt(struct cchar_unit_proto_t *unit)
{
	struct cchar_unit_term_t *term = (struct cchar_unit_term_t*) unit;
	return atomic_load_explicit(&term->spec, memory_order_acquire) ? true : false;
}

// -----------------------------------------------------------------------
int cchar_term_read(struct cchar_unit_term_t *unit, uint16_t *r_arg)
{
	int data = fdb_read(unit->term);

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
int cchar_term_write(struct cchar_unit_term_t *unit, uint16_t *r_arg)
{
	int res = IO_OK;
	char data = *r_arg & 0xff;

	if (fdb_write(unit->term, data) < 0) {
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
int cchar_term_disconnect(struct cchar_unit_term_t *unit)
{
	LOG(L_TERM, "Command: disconnect");
	fdb_await_read(unit->term);
	return IO_OK;
}

// -----------------------------------------------------------------------
int cchar_term_cmd(struct cchar_unit_proto_t *unit, int dir, int cmd, uint16_t *r_arg)
{
	struct cchar_unit_term_t *u = (struct cchar_unit_term_t *) unit;
	if (dir == IO_IN) {
		switch (cmd) {
		case CCHAR_TERM_CMD_SPU:
			LOG(L_TERM, "Command: SPU");
			break;
		case CCHAR_TERM_CMD_READ:
			return cchar_term_read(u, r_arg);
		default:
			LOG(L_TERM, "Unknown IN command: %i", cmd);
			break;
		}
	} else {
		switch (cmd) {
		case CCHAR_TERM_CMD_RESET:
			cchar_term_reset(unit);
			break;
		case CCHAR_TERM_CMD_DISCONNECT:
			return cchar_term_disconnect(u);
		case CCHAR_TERM_CMD_WRITE:
			return cchar_term_write(u, r_arg);
		default:
			LOG(L_TERM, "Unknown OUT command: %i", cmd);
			break;
		}
	}
	return IO_OK;
}

// vim: tabstop=4 shiftwidth=4 autoindent
