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

#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <inttypes.h>
#include <stdio.h>
#include <unistd.h>

#include "em400.h"
#include "emulog.h"
#include "errors.h"
#include "mem/mem.h"
#include "io/cchar.h"
#include "io/cchar_term.h"
#include "io/term.h"

#define UNIT ((struct cchar_unit_term_t *)(unit))

// -----------------------------------------------------------------------
struct cchar_unit_proto_t * cchar_term_create(struct cfg_arg_t *args)
{
	char *type = NULL;
	int port;
	int res;

	struct cchar_unit_term_t *unit = calloc(1, sizeof(struct cchar_unit_term_t));
	if (!unit) {
		goto fail;
	}

	res = cfg_args_decode(args, "s", &type);
	if (res != E_OK) {
		gerr = res;
		goto fail;
	}

	if (!strcasecmp(type, "tcp")) {
		res = cfg_args_decode(args, "si", &type, &port);
		if (res != E_OK) {
			gerr = res;
			goto fail;
		}
		unit->term = term_open_tcp(port, 100);
		if (!unit->term) {
			gerr = E_TERM;
			goto fail;
		}
	} else if (!strcasecmp(type, "console")) {
		if (em400_console == CONSOLE_DEBUGGER) {
			gerr = E_TERM_CONSOLE_DEBUG;
			goto fail;
		} else if (em400_console == CONSOLE_TERMINAL) {
			gerr = E_TERM_CONSOLE_TERM;
			goto fail;
		} else {
			em400_console = CONSOLE_TERMINAL;
			unit->term = term_open_console();
			if (!unit->term) {
				gerr = E_TERM;
				goto fail;
			}
		}
		printf("Console connected as system terminal.\n");
	} else {
		gerr = E_TERM_UNKNOWN;
		goto fail;
	}

	unit->buf = malloc(TERM_BUF_LEN);
	if (!unit->buf) {
		goto fail;
	}

	eprint("      Terminal (%s), port: %i\n", type, port);

	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutex_init(&unit->buf_mutex, &attr);

	res = pthread_create(&unit->worker, NULL, cchar_term_worker, (void *)unit);
	if (res != 0) {
		goto fail;
	}

	return (struct cchar_unit_proto_t *) unit;

fail:
	cchar_term_shutdown((struct cchar_unit_proto_t*) unit);
	return NULL;
}

// -----------------------------------------------------------------------
void cchar_term_shutdown(struct cchar_unit_proto_t *unit)
{
	if (unit) {
		free(UNIT->buf);
		if (UNIT->term) term_close(UNIT->term);
		free(UNIT);
	}
}

// -----------------------------------------------------------------------
void cchar_term_reset(struct cchar_unit_proto_t *unit)
{
}

// -----------------------------------------------------------------------
void cchar_term_queue_char(struct cchar_unit_proto_t *unit, char data)
{
	pthread_mutex_lock(&UNIT->buf_mutex);

	UNIT->buf[UNIT->buf_wpos] = data;
	UNIT->buf_len++;
	if (UNIT->buf_wpos >= TERM_BUF_LEN-2) {
		UNIT->buf_wpos = 0;
	} else {
		UNIT->buf_wpos++;
	}

	if (UNIT->empty_read) {
		cchar_int(unit->chan, unit->num, CCHAR_TERM_INT_READY);
		UNIT->empty_read = 0;
	}

	pthread_mutex_unlock(&UNIT->buf_mutex);
}

// -----------------------------------------------------------------------
void * cchar_term_worker(void *ptr)
{
	struct cchar_unit_proto_t *unit = ptr;
	char data;
	int res;
	static int counter;

	while (1) {
		res = term_read(UNIT->term, &data, 1);
		if (res <= 0) {
			continue;
		}

		if (data == 10) {
			cchar_term_queue_char(unit, 10);
			cchar_term_queue_char(unit, 13);
			counter++;
		} else if (data == 13) {
			continue;
		} else {
			cchar_term_queue_char(unit, data);
		}
	}

	pthread_exit(NULL);
}

// -----------------------------------------------------------------------
int cchar_term_read(struct cchar_unit_proto_t *unit, uint16_t *r_arg)
{
	if (UNIT->buf_len <= 0) {
		pthread_mutex_lock(&UNIT->buf_mutex);
		EMULOG(L_TERM, 50, "buffer empty");
		UNIT->empty_read = 1;
		pthread_mutex_unlock(&UNIT->buf_mutex);
		return IO_EN;
	} else {
		pthread_mutex_lock(&UNIT->buf_mutex);
		char data = UNIT->buf[UNIT->buf_rpos];
#ifdef WITH_DEBUGGER
		if (data >= 32) {
			EMULOG(L_TERM, 50, "buf read: %i (%c)", data, data);
		} else {
			EMULOG(L_TERM, 50, "buf read: %i (#%02x)", data, data);
		}
#endif
		UNIT->buf_len--;
		if (UNIT->buf_rpos >= TERM_BUF_LEN-2) {
			UNIT->buf_rpos = 0;
		} else {
			UNIT->buf_rpos++;
		}
		UNIT->empty_read = 0;
		pthread_mutex_unlock(&UNIT->buf_mutex);
		if (data == 10) {
			return IO_NO;
		} else {
			*r_arg = data;
			return IO_OK;
		}
	}
}

// -----------------------------------------------------------------------
int cchar_term_write(struct cchar_unit_proto_t *unit, uint16_t *r_arg)
{
	char data = *r_arg & 255;
#ifdef WITH_DEBUGGER
	if (data >= 32) {
		EMULOG(L_TERM, 50, "Term write: %i (%c)", data, data);
	} else {
		EMULOG(L_TERM, 50, "Term write: %i (#%02x)", data, data);
	}
#endif
	term_write(UNIT->term, &data, 1);
	return IO_OK;
}

// -----------------------------------------------------------------------
int cchar_term_cmd(struct cchar_unit_proto_t *unit, int dir, int cmd, uint16_t *r_arg)
{
	if (dir == IO_IN) {
		switch (cmd) {
		case CCHAR_TERM_CMD_SPU:
			EMULOG(L_TERM, 1, "TERM: SPU");
			break;
		case CCHAR_TERM_CMD_READ:
			return cchar_term_read(unit, r_arg);
		default:
			EMULOG(L_TERM, 1, "TERM: unknown IN command");
			break;
		}
	} else {
		switch (cmd) {
		case CCHAR_TERM_CMD_RESET:
			EMULOG(L_TERM, 1, "TERM: reset");
			break;
		case CCHAR_TERM_CMD_DISCONNECT:
			EMULOG(L_TERM, 1, "TERM: disconnect");
			break;
		case CCHAR_TERM_CMD_WRITE:
			return cchar_term_write(unit, r_arg);
		default:
			EMULOG(L_TERM, 1, "TERM: unknown OUT command");
			break;
		}
	}
	return IO_OK;
}

// vim: tabstop=4 shiftwidth=4 autoindent
