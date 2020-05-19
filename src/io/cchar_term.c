//  Copyright (c) 2013 Jakub Filipowicz <jakubf@gmail.com>
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
#include <string.h>
#include <strings.h>
#include <inttypes.h>
#include <stdio.h>
#include <unistd.h>

#include "em400.h"
#include "io/defs.h"
#include "io/cchar_term.h"
#include "io/dev/term.h"

#include "log.h"
#include "cfg.h"

#define UNIT ((struct cchar_unit_term_t *)(unit))

// -----------------------------------------------------------------------
struct cchar_unit_proto_t * cchar_term_create(dictionary *cfg, int ch_num, int dev_num)
{
	struct cchar_unit_term_t *unit = (struct cchar_unit_term_t *) calloc(1, sizeof(struct cchar_unit_term_t));
	if (!unit) {
		LOGERR("Failed to allocate memory for terminal: %i.%i", ch_num, dev_num);
		goto fail;
	}

	const char *transport = cfg_fgetstr(cfg, "dev%i.%i:transport", ch_num, dev_num);
	if (!strcasecmp(transport, "tcp")) {
		int port = cfg_fgetint(cfg, "dev%i.%i:port", ch_num, dev_num);
		unit->term = term_open_tcp(port, 100);
		if (!unit->term) {
			LOGERR("Failed to open TCP terminal on port %i.", port);
			goto fail;
		}
		LOG(L_TERM, "Terminal (%s), port: %i", transport, port);

	} else if (!strcasecmp(transport, "serial")) {
		const char * device = cfg_fgetstr(cfg, "dev%i.%i:device", ch_num, dev_num);
		int speed = cfg_fgetint(cfg, "dev%i.%i:speed", ch_num, dev_num);
		unit->term = term_open_serial(device, speed, 100);
		if (!unit->term) {
			LOGERR("Failed to open serial terminal at %s, speed: %i).", device, speed);
			goto fail;
		}
		LOG(L_TERM, "Terminal (%s), device: %s, speed: %i", transport, device, speed);

	} else if (!strcasecmp(transport, "console")) {
		if (em400_console == CONSOLE_DEBUGGER) {
			LOGERR("Failed to initialize console terminal; console is being used by the debugger.");
			goto fail;
		} else if (em400_console == CONSOLE_TERMINAL) {
			LOGERR("Failed to initialize console terminal; console is being used by another terminal.");
			goto fail;
		} else {
			em400_console = CONSOLE_TERMINAL;
			unit->term = term_open_console();
			if (!unit->term) {
				LOGERR("Failed to initialize console.");
				goto fail;
			}
		}
		fprintf(stderr, "Console connected as system terminal.\n");

	} else {
		LOGERR("Unknown terminal transport: %s.", transport);
		goto fail;
	}

	unit->buf = (char *) malloc(TERM_BUF_LEN);
	if (!unit->buf) {
		LOGERR("Failed to allocate memory for terminal buffer.");
		goto fail;
	}

	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutex_init(&unit->buf_mutex, &attr);

	int res = pthread_create(&unit->worker, NULL, cchar_term_worker, (void *)unit);
	if (res != 0) {
		LOGERR("Failed to spawn terminal thread.");
		goto fail;
	}

	pthread_setname_np(unit->worker, "term");

	return (struct cchar_unit_proto_t *) unit;

fail:
	cchar_term_shutdown((struct cchar_unit_proto_t*) unit);
	return NULL;
}

// -----------------------------------------------------------------------
void cchar_term_shutdown(struct cchar_unit_proto_t *unit)
{
	struct cchar_unit_term_t *u = (struct cchar_unit_term_t *) unit;
	if (u) {
		pthread_cancel(u->worker);
		pthread_join(u->worker, NULL);
		free(u->buf);
		if (u->term) term_close(u->term);
		free(u);
	}
}

// -----------------------------------------------------------------------
void cchar_term_reset(struct cchar_unit_proto_t *unit)
{
}

// -----------------------------------------------------------------------
void cchar_term_queue_char(struct cchar_unit_proto_t *unit, char data)
{
	int report_int = 0;

	LOG(L_TERM, "enqueue char: #%02x", data);
	pthread_mutex_lock(&UNIT->buf_mutex);

	UNIT->buf[UNIT->buf_wpos] = data;
	UNIT->buf_len++;
	if (UNIT->buf_wpos >= TERM_BUF_LEN-2) {
		UNIT->buf_wpos = 0;
	} else {
		UNIT->buf_wpos++;
	}

	if (UNIT->empty_read) {
		UNIT->empty_read = 0;
		report_int = 1;
	}

	pthread_mutex_unlock(&UNIT->buf_mutex);

	if (report_int) {
		LOG(L_TERM, "new char available, sending interrupt");
		cchar_int(unit->chan, unit->num, CCHAR_TERM_INT_READY);
	}
}

// -----------------------------------------------------------------------
void * cchar_term_worker(void *ptr)
{
	struct cchar_unit_proto_t *unit = (struct cchar_unit_proto_t *) ptr;
	char data;
	int res;

	while (1) {
		res = term_read(UNIT->term, &data, 1);
		usleep(1000);
		if ((res <= 0) || (data == 10)) {
			continue;
		}
		cchar_term_queue_char(unit, data);
	}

	pthread_exit(NULL);
}

// -----------------------------------------------------------------------
int cchar_term_read(struct cchar_unit_proto_t *unit, uint16_t *r_arg)
{
	pthread_mutex_lock(&UNIT->buf_mutex);
	if (UNIT->buf_len <= 0) {
		LOG(L_TERM, "buffer empty");
		UNIT->empty_read = 1;
		pthread_mutex_unlock(&UNIT->buf_mutex);
		return IO_EN;
	} else {
		uint8_t data = UNIT->buf[UNIT->buf_rpos];
		if (LOG_ENABLED) {
			if (data >= 32) {
				LOG(L_TERM, "%i (%c)", data, data);
			} else {
				LOG(L_TERM, "%i (#%02x)", data, data);
			}
		}

		UNIT->buf_len--;
		if (UNIT->buf_rpos >= TERM_BUF_LEN-2) {
			UNIT->buf_rpos = 0;
		} else {
			UNIT->buf_rpos++;
		}
		pthread_mutex_unlock(&UNIT->buf_mutex);
		*r_arg = data;
		return IO_OK;
	}
}

// -----------------------------------------------------------------------
int cchar_term_write(struct cchar_unit_proto_t *unit, uint16_t *r_arg)
{
	char data = *r_arg & 255;
	if (LOG_ENABLED) {
		if (data >= 32) {
			LOG(L_TERM, "%i (%c)", data, data);
		} else {
			LOG(L_TERM, "%i (#%02x)", data, data);
		}
	}
	term_write(UNIT->term, &data, 1);
	return IO_OK;
}

// -----------------------------------------------------------------------
int cchar_term_cmd(struct cchar_unit_proto_t *unit, int dir, int cmd, uint16_t *r_arg)
{
	if (dir == IO_IN) {
		switch (cmd) {
		case CCHAR_TERM_CMD_SPU:
			LOG(L_TERM, "command: SPU");
			break;
		case CCHAR_TERM_CMD_READ:
			return cchar_term_read(unit, r_arg);
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
			return cchar_term_write(unit, r_arg);
		default:
			LOG(L_TERM, "unknown OUT command: %i", cmd);
			break;
		}
	}
	return IO_OK;
}

// vim: tabstop=4 shiftwidth=4 autoindent
