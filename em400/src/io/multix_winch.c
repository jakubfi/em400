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

#include "errors.h"
#include "cpu/memory.h"

#include "io/multix_winch.h"
#include "io/devices/winch.h"

#define UNIT ((struct mx_unit_winch_t *)(unit))

// -----------------------------------------------------------------------
struct mx_unit_proto_t * mx_winch_create(struct cfg_arg_t *args)
{
	int cyl, head, sect, ssize;
	char *image_name = NULL;
	int res = cfg_args_decode(args, "iiiis", &cyl, &head, &sect, &ssize, &image_name);
	if (res != E_OK) {
		gerr = res;
		return NULL;
	}

	eprint("      Winchester: cyl=%i, head=%i, sectors=%i, spt=%i, image=%s\n", cyl, head, sect, ssize, image_name);

	struct winchester_t *winchester = winch_create(cyl, head, sect, ssize, image_name);
	if (!winchester) {
		free(image_name);
		return NULL;
	}

	struct mx_unit_winch_t *unit = mx_winch_create_internal(winchester);
	if (!unit) {
		free(image_name);
		gerr = E_ALLOC;
		return NULL;
	}

	free(image_name);
	return (struct mx_unit_proto_t *) unit;
}

// -----------------------------------------------------------------------
struct mx_unit_winch_t * mx_winch_create_internal(struct winchester_t *winchester)
{
	struct mx_unit_winch_t *unit = calloc(1, sizeof(struct mx_unit_winch_t));
	if (unit) {
		mx_winch_connect(unit, winchester);
	}
	return unit;
}

// -----------------------------------------------------------------------
void mx_winch_connect(struct mx_unit_winch_t *unit, struct winchester_t *winchester)
{
	UNIT->winchester = winchester;
}

// -----------------------------------------------------------------------
void mx_winch_disconnect(struct mx_unit_winch_t *unit)
{
	winch_shutdown(UNIT->winchester);
	UNIT->winchester = NULL;
}

// -----------------------------------------------------------------------
void mx_winch_shutdown(struct mx_unit_proto_t *unit)
{
	mx_winch_disconnect(UNIT);
	free(UNIT);
}

// -----------------------------------------------------------------------
void mx_winch_reset(struct mx_unit_proto_t *unit)
{

}

// -----------------------------------------------------------------------
int mx_winch_cfg_phy(struct mx_unit_proto_t *unit, struct mx_cf_sc_pl *cfg_phy)
{
	return E_OK;
}

// -----------------------------------------------------------------------
int mx_winch_cfg_log(struct mx_unit_proto_t *unit, struct mx_cf_sc_ll *cfg_log)
{
	return E_OK;
}

// -----------------------------------------------------------------------
int mx_winch_cmd(struct mx_unit_proto_t *unit, int dir, uint16_t n, uint16_t *r)
{
	return IO_OK;
}

// -----------------------------------------------------------------------
struct mx_winch_cf_t * mx_winch_cf_t_decode(int addr)
{
	uint16_t data;
	struct mx_winch_cf_t *cf = calloc(1, sizeof(struct mx_winch_cf_t));
	if (!cf) {
		return NULL;
	}

	data = MEMB(0, addr);
	cf->oper = (data & 0b0000001100000000) >> 8;
	switch (cf->oper) {
		case MX_WINCH_FORMAT_SPARE:
			cf->format = calloc(1, sizeof(struct mx_winch_cf_format));
			data = MEMB(0, addr+1);
			cf->format->sector_map = data;
			data = MEMB(0, addr+2);
			cf->format->start_sector = data << 16;
			data = MEMB(0, addr+3);
			cf->format->start_sector += data;
			break;
		case MX_WINCH_FORMAT:
			// nothing else
			break;
		case MX_WINCH_READ:
		case MX_WINCH_WRITE:
			cf->transmit = calloc(1, sizeof(struct mx_winch_cf_transmit));
			cf->transmit->ign_crc		= (data & 0b0001000000000000) >> 12;
			cf->transmit->sector_fill	= (data & 0b0000100000000000) >> 11;
			cf->transmit->watch_eof		= (data & 0b0000010000000000) >> 10;
			cf->transmit->cpu			= (data & 0b0000000000010000) >> 4;
			cf->transmit->nb			= (data & 0b0000000000001111);
			data = MEMB(0, addr+1);
			cf->transmit->addr = data;
			data = MEMB(0, addr+2);
			cf->transmit->len = data+1;
			data = MEMB(0, addr+3);
			cf->transmit->sector = data << 16;
			data = MEMB(0, addr+4);
			cf->transmit->sector += data;
			break;
		case MX_WINCH_PARK:
			cf->park = calloc(1, sizeof(struct mx_winch_cf_park));
			data = MEMB(0, addr+4);
			cf->park->cylinder = data;
			break;
		default:
			break;
	}

	return cf;
}

// -----------------------------------------------------------------------
void mx_winch_cf_t_free(struct mx_winch_cf_t *cf)
{
	if (cf->format) free(cf->format);
	if (cf->park) free(cf->park);
	if (cf->transmit) free(cf->transmit);
	free(cf);
}

// vim: tabstop=4 shiftwidth=4 autoindent
