//  Copyright (c) 2012-2013 Jakub Filipowicz <jakubf@gmail.com>
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

#ifndef DEBUGGER_DASM_ISET_H
#define DEBUGGER_DASM_ISET_H

#include <inttypes.h>
#include <stdbool.h>

struct dasm_opdef {
	uint16_t opcode;				// basic/extended opcode
	char *mnemo;					// mnemonic (for disassembler)
	_Bool twoword;					// can occupy 2 words?
	int (*extop_fun)(int);			// function to get extended op
	struct dasm_opdef * e_opdef;	// pointer to extop def table
	char* d_format;					// disassembler format
	char* t_format;					// translator format
};

// basic opcodes
extern struct dasm_opdef dasm_iset[];

// sub-opcodes (2nd level)
extern struct dasm_opdef dasm_iset_37[];
extern struct dasm_opdef dasm_iset_70[];
extern struct dasm_opdef dasm_iset_71[];
extern struct dasm_opdef dasm_iset_72[];
extern struct dasm_opdef dasm_iset_73[];
extern struct dasm_opdef dasm_iset_74[];
extern struct dasm_opdef dasm_iset_75[];
extern struct dasm_opdef dasm_iset_76[];
extern struct dasm_opdef dasm_iset_77[];

#endif

// vim: tabstop=4
