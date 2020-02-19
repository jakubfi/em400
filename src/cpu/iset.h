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

// various execution phase time values
#define TIME_MEM 1050			// memory access
#define TIME_PREMOD 300			// premodification
#define TIME_BMOD 300			// B-modification
#define TIME_DMOD 1050			// D-modification
#define TIME_SHIFT 100			// single SHC shift
#define TIME_MEM_ARG 850		// memory argument
#define TIME_P 1400				// skip (P) time
#define TIME_NOANS_IF 15000		// timeout on the interface
#define TIME_INT_SERVE 9000		// interrupt serve

enum opcode_flags {
	OP_FL_NONE			= 0,
	OP_FL_ARG_NORM		= 0x1,	// instruction with normal argument
	OP_FL_ARG_SHORT		= 0x2,	// instruction with short argument
	OP_FL_ILLEGAL		= 0x4,	// illegal instruction
	OP_FL_USR_ILLEGAL	= 0x8,	// instruction illegal in user mode
	OP_FL_IO			= 0x10,	// I/O instruction
};

typedef void (*opfun)();

struct iset_opcode {
	unsigned flags;			// opcode flags
	opfun fun;				// instruction function
	unsigned jmp_nef_mask;	// jump ineffectiveness mask (R0 is checked through this mask)
	unsigned jmp_nef_result;// jump effectiveness result (result for the jump to be effective)
	unsigned time;			// instruction time in ns
};

struct iset_instruction {
	uint16_t opcode;		// instruction opcode (and extended opcode)
	uint16_t var_mask;		// variable bits mask
	struct iset_opcode op;	// opcode definition
};

int iset_build(struct iset_opcode **op_tab, int cpu_user_io_illegal);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
