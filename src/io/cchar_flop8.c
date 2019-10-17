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
#include "io/cchar_flop8.h"

#include "log.h"

#define UNIT ((struct cchar_unit_flop8_t *)(unit))

// -----------------------------------------------------------------------
struct cchar_unit_proto_t * cchar_flop8_create(struct cfg_arg *args)
{
	int res;

	struct cchar_unit_flop8_t *unit = (struct cchar_unit_flop8_t *) calloc(1, sizeof(struct cchar_unit_flop8_t));
	if (!unit) {
		LOGERR("Failed to allocate memory for unit: %s.", args->text);
		goto fail;
	}

	struct cfg_arg *a = args;
	int id = 0;

	while (id < 4) {
		if (a) {
			res = cfg_args_decode(a, "s", &unit->image[id]);
			if (res == E_OK) {
				unit->f[id] = fopen(unit->image[id], "r");
				if (unit->f[id]) {
					LOG(L_FLOP, "Drive %i: \"%s\".", id, unit->image[id]);
				} else {
					LOG(L_FLOP, "Failed to open image: \"%s\".", unit->image[id]);
				}
			} else {
				LOGERR("Failed to parse image name: %s.", a->text);
				goto fail;
			}
		} else {
			break;
		}

		a = a->next;
		id++;
	}

	unit->track = 1;
	unit->sector = 1;
	unit->disk = 0;
	unit->byte = 0;

	return (struct cchar_unit_proto_t *) unit;

fail:
	cchar_flop8_shutdown((struct cchar_unit_proto_t*) unit);
	return NULL;
}

// -----------------------------------------------------------------------
void cchar_flop8_shutdown(struct cchar_unit_proto_t *unit)
{
	struct cchar_unit_flop8_t *u = (struct cchar_unit_flop8_t *) unit;
	if (u) {
		for (int i=0 ; i<4 ; i++) {
			if (u->f[i]) fclose(u->f[i]);
			if (u->image[i]) free(u->image[i]);
		}
		free(u);
	}
}

// -----------------------------------------------------------------------
void cchar_flop8_reset(struct cchar_unit_proto_t *unit)
{
	struct cchar_unit_flop8_t *u = (struct cchar_unit_flop8_t *) unit;
	u->track = 1;
	u->sector = 1;
	u->disk = 0;
	u->byte = 0;
}

// -----------------------------------------------------------------------
static int cchar_flop8_read(struct cchar_unit_proto_t *unit, uint16_t *r_arg)
{
	struct cchar_unit_flop8_t *u = (struct cchar_unit_flop8_t *) unit;
	int res;
	uint8_t data;
	int lba;

	lba = (u->track-1) * 26 + (u->sector-1);
	res = fseek(u->f[u->disk], lba*128 + u->byte, SEEK_SET);
	if (res != 0) {
		LOG(L_FLOP, "Failed floppy seek");
	}
	res = fread(&data, 1, 1, u->f[u->disk]);
	if (res != 1) {
		LOG(L_FLOP, "Failed floppy read");
	}

	*r_arg = data;

	u->byte++;
	if (u->byte >= 128) {
		u->byte = 0;
		u->sector++;
		LOG(L_FLOP, "sector end, next sector: %i", u->sector);
		if (u->sector > 26) {
			u->sector = 1;
			u->track++;
			LOG(L_FLOP, "track end, next track: %i", u->track);
			if (u->track > 73) {
				u->track = 1;
				LOG(L_FLOP, "disk end");
			}
		}
	}

	return IO_OK;
}

// -----------------------------------------------------------------------
static int cchar_flop8_write(struct cchar_unit_proto_t *unit, uint16_t *r_arg)
{

	return IO_OK;
}

// -----------------------------------------------------------------------
static int cchar_flop8_seek(struct cchar_unit_proto_t *unit, uint16_t *r_arg)
{
	struct cchar_unit_flop8_t *u = (struct cchar_unit_flop8_t *) unit;

	uint8_t disk = (*r_arg >> 13) & 0b111;
	switch (disk) {
		case 0b000: u->disk = 0; break;
//		case 0b001: u->disk = 1; break;
//		case 0b100: u->disk = 2; break;
//		case 0b101: u->disk = 3; break;
		default: u->disk = 1; break;
	}
	u->sector = *r_arg & 0b11111;
	u->track = (*r_arg >> 5) & 0b1111111;
	u->byte = 0;
	LOG(L_FLOP, "Seek: %i/%i", u->track, u->sector);
	return IO_OK;
}

// -----------------------------------------------------------------------
int cchar_flop8_cmd(struct cchar_unit_proto_t *unit, int dir, int cmd, uint16_t *r_arg)
{
	if (dir == IO_IN) {
		switch (cmd) {
		case CCHAR_FLOP8_CMD_SPU:
			LOG(L_FLOP, "command: SPU");
			break;
		case CCHAR_FLOP8_CMD_READ:
			return cchar_flop8_read(unit, r_arg);
		default:
			LOG(L_FLOP, "unknown IN command: %i", cmd);
			break;
		}
	} else {
		switch (cmd) {
		case CCHAR_FLOP8_CMD_RESET:
			LOG(L_FLOP, "command: reset");
			cchar_flop8_reset(unit);
			break;
		case CCHAR_FLOP8_CMD_DISCONNECT:
			LOG(L_FLOP, "command: disconnect");
			cchar_flop8_reset(unit);
			break;
		case CCHAR_FLOP8_CMD_WRITE:
			return cchar_flop8_write(unit, r_arg);
		case CCHAR_FLOP8_CMD_SEEK:
			return cchar_flop8_seek(unit, r_arg);
		default:
			LOG(L_FLOP, "unknown OUT command: %i", cmd);
			break;
		}
	}
	return IO_OK;
}

// vim: tabstop=4 shiftwidth=4 autoindent
