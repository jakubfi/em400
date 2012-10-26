//  Copyright (c) 2012 Jakub Filipowicz <jakubf@gmail.com>
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
#include <stdbool.h>

#include "registers.h"

struct mjc400_opdef {
	uint16_t opcode;				// basic/extended opcode
	char *mnemo;					// mnemonic (for disassembler)
	_Bool nl;						// legal in user mode?
	_Bool twoword;					// can occupy 2 words?
	int (*op_fun)();				// instruction execution function
	int (*extop_fun)(int);			// function to get extended op
	struct mjc400_opdef * e_opdef;	// pointer to extop def table
	char* d_format;					// disassembler format
	char* t_format;					// translator format
};

// basic opcodes jump table
extern struct mjc400_opdef mjc400_iset[];

// macros to access sub-opcodes
#define EXT_OP_37(x) _A(x)
#define EXT_OP_70(x) _A(x)
#define EXT_OP_71(x) ((x & 0b0000001100000000) >> 8) // [D,A0]
#define EXT_OP_72(x) (((x & 0b0000001000000000) >> 4) | (x & 0b0000000000011111)) // [D,B1,B2,C] (b0 is always 0)
#define EXT_OP_73(x) (((x & 0b0000001111000000) >> 3) | (x & 0b0000000000000111)) // [D,A,C0,C1,C2] (but not all used in each instruction)
#define EXT_OP_74(x) _A(x)
#define EXT_OP_75(x) _A(x)
#define EXT_OP_76(x) _A(x)
#define EXT_OP_77(x) _A(x)

// sub-opcodes (2nd level) jump tables
extern struct mjc400_opdef mjc400_iset_37[];
extern struct mjc400_opdef mjc400_iset_70[];
extern struct mjc400_opdef mjc400_iset_71[];
extern struct mjc400_opdef mjc400_iset_72[];
extern struct mjc400_opdef mjc400_iset_73[];
extern struct mjc400_opdef mjc400_iset_74[];
extern struct mjc400_opdef mjc400_iset_75[];
extern struct mjc400_opdef mjc400_iset_76[];
extern struct mjc400_opdef mjc400_iset_77[];

#endif

// vim: tabstop=4
