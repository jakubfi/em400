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

#ifndef ISET_H
#define ISET_H

#include <inttypes.h>

typedef void (*opfun)();

struct em400_op {
	int norm_arg;			// has normal argument?
	int short_arg;			// has short argument?
	int user_illegal;		// is illegal in user mode?
	opfun fun;				// instruction function
};

struct em400_instr {
	uint16_t opcode;		// instruction opcode (and extended opcode)
	uint16_t var_mask;		// variable bits mask
	struct em400_op op;		// opcode definition
};

// opcode table (instruction decoder decision table)
extern struct em400_op *em400_op_tab[0x10000];

// instruction list
extern struct em400_instr em400_ilist_mera400[];
extern struct em400_instr em400_instr_in_legal;
extern struct em400_instr em400_instr_ou_legal;
extern struct em400_instr em400_instr_sint;
extern struct em400_instr em400_instr_sind;
extern struct em400_instr em400_instr_cron;
extern struct em400_instr em400_instr_illegal;

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
