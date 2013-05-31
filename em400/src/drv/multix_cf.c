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

#include <inttypes.h>
#include <stdlib.h>

#include "errors.h"
#include "cpu/memory.h"
#include "drv/multix.h"
#include "drv/multix_cf.h"

// -----------------------------------------------------------------------
int mx_decode_cf_sc(int addr, struct mx_cf_sc *cf)
{
	uint16_t data;

	// --- word 0 - header ---
	data = MEMB(0, addr);
	cf->pl_desc_count = (data & 0b1111111100000000) >> 8;
	cf->ll_desc_count = (data & 0b0000000011111111);

	// --- word 1 - return field ---
	cf->retf = mem_ptr(0, addr+1);

	if (cf->pl_desc_count <= 0) {
		// missing physical line description
		return E_CF;
	}

	if (cf->ll_desc_count <= 0) {
		// missing logical line descroption
		return E_CF;
	}

	cf->pl = calloc(cf->pl_desc_count, sizeof(struct mx_cf_sc_pl));
	cf->ll = calloc(cf->ll_desc_count, sizeof(struct mx_cf_sc_ll));

	if (!cf->pl || !cf->ll) {
		// cannot allocate memory, treated as communication error
		return E_ALLOC;
	}

	// --- physical lines, 1 word each ---
	for (int pln=0 ; pln<cf->pl_desc_count ; pln++) {
		data = MEMB(0, addr+2+pln);
		cf->pl[pln].dir =		(data & 0b1110000000000000) >> 13;
		cf->pl[pln].used =		(data & 0b0001000000000000) >> 12;
		cf->pl[pln].dev_type =	(data & 0b0000111100000000) >> 8;
		cf->pl[pln].count =		(data & 0b0000000000011111) + 1;
	}

	// --- logical lines, 4 words each ---
	for (int lln=0 ; lln<cf->ll_desc_count ; lln++) {
		data = MEMB(0, addr+2+cf->pl_desc_count+(lln*4));
		cf->ll[lln].proto = (data & 0b1111111100000000) >> 8;
		cf->ll[lln].pl_id = (data & 0b0000000011111111);
		switch (cf->ll[lln].proto) {
			case 6: // Winchester
				cf->ll[lln].winch = calloc(1, sizeof(struct mx_ll_winch));
				if (!cf->ll[lln].winch) {
					return E_ALLOC;
				}
				data = MEMB(0, addr+2+cf->pl_desc_count+(lln*4)+1);
				cf->ll[lln].winch->type =			(data & 0b1111111100000000) >> 8;
				cf->ll[lln].winch->format_protect =	(data & 0b0000000011111111);
				break;
			case 7: // magnetic tape
				// MT protocol changes meaning of pl_id and adds formatter field
				cf->ll[lln].pl_id &= 0b00011111;
				cf->ll[lln].formatter = (cf->ll[lln].pl_id & 0b10000000) >> 7;
				break;
			case 8: // floppy disk
				cf->ll[lln].floppy = calloc(1, sizeof(struct mx_ll_floppy));
				if (!cf->ll[lln].floppy) {
					return E_ALLOC;
				}
				data = MEMB(0, addr+2+cf->pl_desc_count+(lln*4)+1);
				cf->ll[lln].floppy->type =				(data & 0b1111111100000000) >> 8;
				cf->ll[lln].floppy->format_protect =	(data & 0b0000000011111111);
				break;
			default: // unknown protocol
				return E_CF;
		}
	}

	return E_OK;
}

// -----------------------------------------------------------------------
void mx_free_cf_sc(struct mx_cf_sc *cf)
{
	if (cf->pl) {
		free(cf->pl);
	}

	if (cf->ll) {
		for (int i=0 ; i<cf->ll_desc_count ; i++) {
			if (cf->ll[i].winch) {
				free(cf->ll[i].winch);
			}
			if (cf->ll[i].floppy) {
				free(cf->ll[i].floppy);
			}
		}
		free(cf->ll);
	}

	free(cf);
}

// -----------------------------------------------------------------------
int mx_decode_cf_winch_t(int addr, struct mx_cf_winch_t *cf)
{
	uint16_t data;

	data = MEMB(0, addr);
	cf->oper = (data & 0b0000001100000000) >> 8;
	switch (cf->oper) {
		case MX_WINCH_FORMAT_SPARE:
			cf->format = calloc(1, sizeof(struct mx_cf_winch_format));
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
			cf->transmit = calloc(1, sizeof(struct mx_cf_winch_transmit));
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
			cf->park = calloc(1, sizeof(struct mx_cf_winch_park));
			data = MEMB(0, addr+4);
			cf->park->cylinder = data;
			break;
		default:
			break;
	}

	return E_OK;
}

// -----------------------------------------------------------------------
void mx_free_cf_winch_t(struct mx_cf_winch_t *cf)
{
	if (cf->format) free(cf->format);
	if (cf->park) free(cf->park);
	if (cf->transmit) free(cf->transmit);
	free(cf);
}

// vim: tabstop=4
