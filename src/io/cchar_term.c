//  Copyright (c) 2013-2020 Jakub Filipowicz <jakubf@gmail.com>
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
#include <stdio.h>

#include "em400.h"
#include "io/defs.h"
#include "io/cchar_term.h"
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

	unit->ignore_lf = cfg_fgetbool(cfg, "dev%i.%i:ignore_lf", ch_num, dev_num);
	const char *transport = cfg_fgetstr(cfg, "dev%i.%i:transport", ch_num, dev_num);
	const int speed = cfg_fgetint(cfg, "dev%i.%i:speed", ch_num, dev_num);
	const char * device = cfg_fgetstr(cfg, "dev%i.%i:device", ch_num, dev_num);
	const int port = cfg_fgetint(cfg, "dev%i.%i:port", ch_num, dev_num);

	if (!strcasecmp(transport, "tcp")) {
		if (!port) {
			LOGERR("TCP terminal needs port to be set.");
			goto fail;
		}
		unit->term = fdb_open_tcp(port);
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
	fdb_set_speed(unit->term, speed);

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
	struct cchar_unit_term_t *u = (struct cchar_unit_term_t *) unit;
	fdb_reset(u->term);
}

// -----------------------------------------------------------------------
int fdb_callback(void *ctx, int condition)
{
	struct cchar_unit_proto_t *unit = (struct cchar_unit_proto_t *) ctx;

	switch (condition) {
		case FDB_READY:
			LOG(L_TERM, "line ready, sending interrupt");
			cchar_int(unit->chan, unit->num, CCHAR_TERM_INT_READY);
			break;
		case FDB_LOST:
			LOG(L_TERM, "character lost, transmission too slow");
			cchar_int(unit->chan, unit->num, CCHAR_TERM_INT_TOO_SLOW);
		default:
			break;
	}
	return 0;
}

// -----------------------------------------------------------------------
int cchar_term_read(struct cchar_unit_term_t *unit, uint16_t *r_arg)
{
	int data = fdb_read(unit->term);

	if (data < 0) {
		LOG(L_TERM, "empty read");
		return IO_EN;
	}

	if (data >= 32) {
		LOG(L_TERM, "terminal read: %i (%c)", data, data);
	} else {
		LOG(L_TERM, "terminal read: %i (#%02x)", data, data);
	}

	*r_arg = data;
	return IO_OK;
}

// -----------------------------------------------------------------------
int cchar_term_write(struct cchar_unit_term_t *unit, uint16_t *r_arg)
{
	char data = *r_arg & 0xff;

	if (fdb_write(unit->term, data) < 0) {
		LOG(L_TERM, "transmitter busy");
		return IO_EN;
	}

	if (data >= 32) {
		LOG(L_TERM, "terminal write: %i (%c)", data, data);
	} else {
		LOG(L_TERM, "terminal write: %i (#%02x)", data, data);
	}

	return IO_OK;
}

// -----------------------------------------------------------------------
int cchar_term_cmd(struct cchar_unit_proto_t *unit, int dir, int cmd, uint16_t *r_arg)
{
	struct cchar_unit_term_t *u = (struct cchar_unit_term_t *) unit;
	if (dir == IO_IN) {
		switch (cmd) {
		case CCHAR_TERM_CMD_SPU:
			LOG(L_TERM, "command: SPU");
			break;
		case CCHAR_TERM_CMD_READ:
			return cchar_term_read(u, r_arg);
		default:
			LOG(L_TERM, "unknown IN command: %i", cmd);
			break;
		}
	} else {
		switch (cmd) {
		case CCHAR_TERM_CMD_RESET:
			LOG(L_TERM, "command: reset");
			break;
		case CCHAR_TERM_CMD_DISCONNECT:
			LOG(L_TERM, "command: disconnect");
			break;
		case CCHAR_TERM_CMD_WRITE:
			return cchar_term_write(u, r_arg);
		default:
			LOG(L_TERM, "unknown OUT command: %i", cmd);
			break;
		}
	}
	return IO_OK;
}

// vim: tabstop=4 shiftwidth=4 autoindent
