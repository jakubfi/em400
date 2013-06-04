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

#include "cpu/memory.h"
#include "io/cmem_cf.h"

#include "errors.h"

// -----------------------------------------------------------------------
int cmem_decode_cf_t(int addr, struct cmem_cf_t *cf)
{
	uint16_t data;

	data = MEMB(0, addr);
	cf->cf_len			= (data & 0b0000111100000000) >> 8;
	cf->cpu				= (data & 0b0000000000010000) >> 4;
	cf->nb				= (data & 0b0000000000001111);
	data = MEMB(0, addr+1);
	cf->oper			= (data & 0b0000011000000000) >> 9;
	data = MEMB(0, addr+2);
	cf->len				= data;
	data = MEMB(0, addr+3);
	cf->ign_wrprotect	= (data & 0b1000000000000000) >> 15;
	cf->ign_defects		= (data & 0b0100000000000000) >> 14;
	cf->ign_key			= (data & 0b0010000000000000) >> 13;
	cf->ign_eof			= (data & 0b0001000000000000) >> 12;
	cf->cyl				= (data & 0b0000000111111111);
	data = MEMB(0, addr+4);
	cf->platter			= (data & 0b0000010000000000) >> 10;
	cf->head			= (data & 0b0000000100000000) >> 8;
	cf->sector			= (data & 0b0000000000001111);
	data = MEMB(0, addr+5);
	cf->key				= data;
	data = MEMB(0, addr+6);
	cf->addr			= data;

	return E_OK;
}


// vim: tabstop=4 shiftwidth=4 autoindent
