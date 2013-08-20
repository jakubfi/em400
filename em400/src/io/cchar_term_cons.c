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
#include <inttypes.h>
#include <stdio.h>

#include "errors.h"
#include "cpu/memory.h"
#include "io/cchar.h"
#include "io/cchar_term_cons.h"

#define UNIT ((struct cchar_unit_term_cons_t *)(unit))

// -----------------------------------------------------------------------
struct cchar_unit_proto_t * cchar_term_cons_create(struct cfg_arg_t *args)
{
	struct cchar_unit_term_cons_t *unit = calloc(1, sizeof(struct cchar_unit_term_cons_t));

	if (!unit) {
		goto fail;
	}

	return (struct cchar_unit_proto_t *) unit;

fail:
	cchar_term_cons_shutdown((struct cchar_unit_proto_t*) unit);
	return NULL;
}

// -----------------------------------------------------------------------
void cchar_term_cons_shutdown(struct cchar_unit_proto_t *unit)
{
	if (unit) {
		free(UNIT);
	}
}

// -----------------------------------------------------------------------
void cchar_term_cons_reset(struct cchar_unit_proto_t *unit)
{
}

// -----------------------------------------------------------------------
int cchar_term_cons_cmd(struct cchar_unit_proto_t *unit, int dir, int cmd, uint16_t *r_arg)
{
	if (dir == IO_IN) {
		switch (cmd) {
		case CCHAR_TERM_CMD_SPU:
			break;
		case CCHAR_TERM_CMD_READ:
			return IO_EN;
			break;
		default:
			break;
		}
	} else {
		switch (cmd) {
		case CCHAR_TERM_CMD_RESET:
			break;
		case CCHAR_TERM_CMD_DISCONNECT:
			break;
		case CCHAR_TERM_CMD_WRITE:
			printf("%c", *r_arg&255);
			fflush(stdout);
			break;
		default:
			break;
		}
	}
	return IO_OK;
}

// vim: tabstop=4 shiftwidth=4 autoindent
