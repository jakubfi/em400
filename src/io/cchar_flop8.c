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
#include "io/defs.h"
#include "io/cchar_flop8.h"
#include "cfg.h"

#include "log.h"

#define DRIVES 4
#define TRACKS 77
#define TRACK_LAST 73
#define SPT 26
#define SECTOR_BYTES 128
#define DISK_SIZE_BYTES (TRACKS * SPT * SECTOR_BYTES)

// commands
enum cchar_flop8_commands {
	// OU
	CCHAR_FLOP8_CMD_RESET		= 0b100000, // reset
	CCHAR_FLOP8_CMD_DISCONNECT	= 0b101000, // disconnect (soft reset)
	CCHAR_FLOP8_CMD_WRITE		= 0b110000, // write
	CCHAR_FLOP8_CMD_SEEK_W		= 0b111000, // seek for write
	CCHAR_FLOP8_CMD_SEEK_WC		= 0b111100, // seek for write+check
	CCHAR_FLOP8_CMD_SEEK_B		= 0b111110, // seek for bad sector
	CCHAR_FLOP8_CMD_SEEK_R		= 0b111010, // seek for read
	// IN
	CCHAR_FLOP8_CMD_SPU			= 0b100000, // check device
	CCHAR_FLOP8_CMD_READ		= 0b101000, // read
};

// floppy drive
enum cchar_flop8_drives {
	CCHAR_FLOP8_DRIVE_0		= 0b000,
	CCHAR_FLOP8_DRIVE_1		= 0b001,
	CCHAR_FLOP8_DRIVE_2		= 0b100,
	CCHAR_FLOP8_DRIVE_3		= 0b101,
};

// floppy side
enum cchar_flop8_sides {
	CCHAR_FLOP8_SIDE_A		= 0,
	CCHAR_FLOP8_SIDE_B		= 1,
};

// interrupts
enum char_flop_interrupts {
	CCHAR_FLOP8_INT_OUTDATED		= 0b00000, // interrupt out of date
	CCHAR_FLOP8_INT_READY			= 0b00001, // device is ready
	CCHAR_FLOP8_INT_HW_ERR			= 0b00010, // hardware error
	CCHAR_FLOP8_INT_SECT_NOT_FOUND	= 0b11010, // sector not found
	CCHAR_FLOP8_INT_CRC_ERR			= 0b01010, // data CRC error
	CCHAR_FLOP8_INT_SECT_BAD		= 0b10010, // sector marked as bad
};

typedef struct cchar_unit_flop8 {
	struct cchar_unit_proto_t proto;
	char *image[DRIVES];
	FILE *f[DRIVES];
	int drive, side, track, sector, byte;
} flop8;

// -----------------------------------------------------------------------
struct cchar_unit_proto_t * cchar_flop8_create(em400_cfg *cfg, int ch_num, int dev_num)
{
	flop8 *unit = (flop8 *) calloc(1, sizeof(flop8));
	if (!unit) {
		LOGERR("Failed to allocate memory for 8-inch floppy: %i.%i", ch_num, dev_num);
		goto fail;
	}

	for (int id=0 ; id<DRIVES ; id++) {
		const char *image = cfg_fgetstr(cfg, "dev%i.%i:image_%i", ch_num, dev_num, id);
		if (!image) continue;

		unit->image[id] = strdup(image);
		if (!unit->image[id]) {
			LOGERR("8-inch floppy image data memory allocation error.");
			goto fail;
		}
		unit->f[id] = fopen(image, "r+b");
		if (unit->f[id]) {
			LOG(L_FLOP, "Drive %i: %s.", id, image);
		} else {
			LOGERR("Failed to open 8-inch floppy image: %s (drive %i).", image, id);
			goto fail;
		}

		fseek(unit->f[id], 0L, SEEK_END);
		int sz = ftell(unit->f[id]);
		if (sz != DISK_SIZE_BYTES) {
			LOGERR("Wrong 8-inch floppy drive %i image '%s' size: %i instead of %i.", id, image, sz, DISK_SIZE_BYTES);
			goto fail;
		}
	}

	unit->drive = 0;
	unit->side = 0;
	unit->track = 1;
	unit->sector = 1;
	unit->byte = 0;

	return (struct cchar_unit_proto_t *) unit;

fail:
	cchar_flop8_shutdown((struct cchar_unit_proto_t*) unit);
	return NULL;
}

// -----------------------------------------------------------------------
void cchar_flop8_shutdown(struct cchar_unit_proto_t *unit)
{
	flop8 *u = (flop8 *) unit;
	if (!u) return;

	for (int i=0 ; i<DRIVES ; i++) {
		if (u->f[i]) fclose(u->f[i]);
		if (u->image[i]) free(u->image[i]);
	}
	free(u);
}

// -----------------------------------------------------------------------
void cchar_flop8_reset(struct cchar_unit_proto_t *unit)
{
	flop8 *u = (flop8 *) unit;
	u->drive = 0;
	u->side = 0;
	u->track = 1;
	u->sector = 1;
	u->byte = 0;
}

// -----------------------------------------------------------------------
static int cchar_flop8_access(struct cchar_unit_proto_t *unit, uint16_t *r_arg, int cmd)
{
	flop8 *u = (flop8 *) unit;
	int res;
	uint8_t data;

	if (!u->f[u->drive]) {
		LOG(L_FLOP, "No image attached to drive %i", u->drive);
		// TODO: send interrupt
		return IO_EN;
	}

	int offset = (u->track * SPT + (u->sector-1)) * SECTOR_BYTES + u->byte;
	res = fseek(u->f[u->drive], offset, SEEK_SET);
	if (res != 0) {
		LOG(L_FLOP, "Image file seek failed");
		// TODO: send interrupt
	}

	if (cmd == CCHAR_FLOP8_CMD_READ) {
		res = fread(&data, 1, 1, u->f[u->drive]);
		if (res != 1) {
			LOG(L_FLOP, "Failed floppy read");
			// TODO: send interrupt
		}
		*r_arg = data;
	} else {
		data = *r_arg & 0xff;
		res = fwrite(&data, 1, 1, u->f[u->drive]);
		if (res != 1) {
			LOG(L_FLOP, "Failed floppy write");
			// TODO: send interrupt
		}
	}

	u->byte++;
	if (u->byte >= SECTOR_BYTES) {
		u->byte = 0;
		u->sector++;
		LOG(L_FLOP, "sector end, next sector: %i", u->sector);
		if (u->sector > SPT) {
			u->sector = 1;
			u->track++;
			LOG(L_FLOP, "track end, next track: %i", u->track);
			if (u->track > TRACK_LAST) {
				u->track = 1;
				LOG(L_FLOP, "disk end");
				// TODO: send interrupt
			}
		}
	}

	return IO_OK;
}

// -----------------------------------------------------------------------
static int cchar_flop8_seek(struct cchar_unit_proto_t *unit, uint16_t *r_arg, int cmd)
{
	flop8 *u = (flop8 *) unit;

	int drive = (*r_arg >> 13) & 0b111;
	u->drive = (drive & 1) | ((drive >> 1) & 2);
	u->side = (*r_arg >> 12) & 1;
	u->track = (*r_arg >> 5) & 0b1111111;
	u->sector = *r_arg & 0b11111;
	u->byte = 0;

	LOG(L_FLOP, "Set new address: drive %i, side: %i, track %i, sector %i", u->drive, u->side, u->track, u->sector);

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
			return cchar_flop8_access(unit, r_arg, CCHAR_FLOP8_CMD_READ);
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
			return cchar_flop8_access(unit, r_arg, CCHAR_FLOP8_CMD_WRITE);
		case CCHAR_FLOP8_CMD_SEEK_R:
		case CCHAR_FLOP8_CMD_SEEK_W:
		case CCHAR_FLOP8_CMD_SEEK_WC:
		case CCHAR_FLOP8_CMD_SEEK_B:
			return cchar_flop8_seek(unit, r_arg, cmd);
		default:
			LOG(L_FLOP, "unknown OUT command: %i", cmd);
			break;
		}
	}
	return IO_OK;
}

// vim: tabstop=4 shiftwidth=4 autoindent
