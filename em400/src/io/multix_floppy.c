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

#include "debugger/log.h"
#include "errors.h"
#include "cpu/memory.h"

#include "io/multix_floppy.h"
#include "io/devices/rawdisk.h"

#define UNIT ((struct mx_unit_floppy_t *)(unit))

// -----------------------------------------------------------------------
struct mx_unit_proto_t * mx_floppy_create(struct cfg_arg_t *args)
{
	int cyl, head, sect, ssize;
	char *image_name = NULL;
	int res = cfg_args_decode(args, "iiiis", &cyl, &head, &sect, &ssize, &image_name);
	if (res != E_OK) {
		gerr = res;
		return NULL;
	}

	eprint("      Floppy: cyl=%i, head=%i, sectors=%i, spt=%i, image=%s\n", cyl, head, sect, ssize, image_name);

	struct rawdisk_t *floppy = rawdisk_create(cyl, head, sect, ssize, image_name);
	if (!floppy) {
		free(image_name);
		return NULL;
	}

	struct mx_unit_proto_t *unit = mx_floppy_create_nodev();
	if (!unit) {
		rawdisk_shutdown(floppy);
		free(image_name);
		gerr = E_ALLOC;
		return NULL;
	}

	mx_floppy_connect(UNIT, floppy);
	free(image_name);
	return unit;
}

// -----------------------------------------------------------------------
struct mx_unit_proto_t * mx_floppy_create_nodev()
{
	struct mx_unit_floppy_t *unit = calloc(1, sizeof(struct mx_unit_floppy_t));
	return (struct mx_unit_proto_t *) unit;
}

// -----------------------------------------------------------------------
void mx_floppy_connect(struct mx_unit_floppy_t *unit, struct rawdisk_t *floppy)
{
	UNIT->floppy = floppy;
}

// -----------------------------------------------------------------------
void mx_floppy_disconnect(struct mx_unit_floppy_t *unit)
{
	rawdisk_shutdown(UNIT->floppy);
	UNIT->floppy = NULL;
}

// -----------------------------------------------------------------------
void mx_floppy_shutdown(struct mx_unit_proto_t *unit)
{
	mx_floppy_disconnect(UNIT);
	free(UNIT);
}

// -----------------------------------------------------------------------
void mx_floppy_reset(struct mx_unit_proto_t *unit)
{

}

// -----------------------------------------------------------------------
int mx_floppy_cfg_phy(struct mx_unit_proto_t *unit, struct mx_cf_sc_pl *cfg_phy)
{
	LOG(D_IO, 10, "MULTIX/floppy (line:%i): configure physical line", unit->phy_num);
	if (unit && cfg_phy) {
		unit->dir = cfg_phy->dir;
		unit->used = 1;
		unit->type = cfg_phy->type;
	} else {
		return E_MX_DECODE;
	}
	return E_OK;
}

// -----------------------------------------------------------------------
int mx_floppy_cfg_log(struct mx_unit_proto_t *unit, struct mx_cf_sc_ll *cfg_log)
{
	LOG(D_IO, 10, "MULTIX/floppy (line:%i): configure logical line", unit->phy_num);
	if (unit && cfg_log && cfg_log->floppy) {
		UNIT->floppy_type = cfg_log->floppy->type;
		UNIT->format_protect = cfg_log->floppy->format_protect;
	} else {
		return E_MX_DECODE;
	}
	return E_OK;
}

// -----------------------------------------------------------------------
int mx_floppy_cmd(struct mx_unit_proto_t *unit, int dircmd, uint16_t addr)
{
	return IO_OK;
}

// -----------------------------------------------------------------------
struct mx_floppy_cf_t * mx_floppy_cf_t_decode(int addr)
{
	uint16_t data;
	struct mx_floppy_cf_t *cf = calloc(1, sizeof(struct mx_floppy_cf_t));
	if (!cf) {
		return NULL;
	}

	data = MEMB(0, addr);
	cf->oper = (data & 0b0000011100000000) >> 8;
	switch (cf->oper) {
		case MX_FLOPPY_FORMAT:
			data = MEMB(0, addr+3);
			cf->format = calloc(1, sizeof(struct mx_floppy_cf_format));
			cf->format->start_sector = data;
			break;
		case MX_FLOPPY_READ:
		case MX_FLOPPY_WRITE:
			cf->transmit = calloc(1, sizeof(struct mx_floppy_cf_transmit));
			cf->transmit->ign_crc		= (data & 0b0001000000000000) >> 12;
			cf->transmit->cpu			= (data & 0b0000000000010000) >> 4;
			cf->transmit->nb			= (data & 0b0000000000001111);
			data = MEMB(0, addr+1);
			cf->transmit->addr = data;
			data = MEMB(0, addr+2);
			cf->transmit->len = data+1;
			data = MEMB(0, addr+3);
			cf->transmit->sector = data << 16;
			break;
		case MX_FLOPPY_BAD_SECTOR:
			data = MEMB(0, addr+3);
			cf->bad_sector = calloc(1, sizeof(struct mx_floppy_cf_bad_sector));
			cf->bad_sector->sector = data;
			break;
		default:
			break;
	}

	return cf;
}

// -----------------------------------------------------------------------
void mx_floppy_cf_t_free(struct mx_floppy_cf_t *cf)
{
	if (cf->format) free(cf->format);
	if (cf->transmit) free(cf->transmit);
	if (cf->bad_sector) free(cf->bad_sector);
	free(cf);
}

// vim: tabstop=4 shiftwidth=4 autoindent
