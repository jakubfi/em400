//  Copyright (c) 2011-2012 Jakub Filipowicz <jakubf@gmail.com>
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
#include <stdbool.h>

typedef struct {
	uint16_t opcode;
	char mnemonic[3];
	_Bool nl;
	int (*op_fun)();

} mjc400_opdef;

extern mjc400_opdef mjc400_iset[];
extern mjc400_opdef mjc400_iset_37[];
extern mjc400_opdef mjc400_iset_70[];
extern mjc400_opdef mjc400_iset_71[];
extern mjc400_opdef mjc400_iset_72[];
extern mjc400_opdef mjc400_iset_73[];
extern mjc400_opdef mjc400_iset_74[];
extern mjc400_opdef mjc400_iset_75[];
extern mjc400_opdef mjc400_iset_76[];
extern mjc400_opdef mjc400_iset_77[];

