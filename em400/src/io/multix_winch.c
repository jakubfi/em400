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

// -----------------------------------------------------------------------
struct unit_proto_t * mx_winch_create(struct cfg_arg_t *args)
{
	struct mx_unit_winch_t *unit = calloc(1, sizeof(struct mx_unit_winch_t));
	return (struct unit_proto_t *) unit;
}

// -----------------------------------------------------------------------
void mx_winch_shutdown(struct unit_proto_t *unit)
{
}

// -----------------------------------------------------------------------
void mx_winch_reset(struct unit_proto_t *unit)
{

}

// -----------------------------------------------------------------------
int mx_winch_cmd(struct unit_proto_t *unit, int dir, uint16_t n, uint16_t *r)
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
			cf->transmit->ign_crc	   = (data & 0b0001000000000000) >> 12;
			cf->transmit->sector_fill   = (data & 0b0000100000000000) >> 11;
			cf->transmit->watch_eof	 = (data & 0b0000010000000000) >> 10;
			cf->transmit->cpu		   = (data & 0b0000000000010000) >> 4;
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
