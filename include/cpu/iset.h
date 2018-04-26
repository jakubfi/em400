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

struct iset_opcode {
	int norm_arg;			// has normal argument?
	int short_arg;			// has short argument?
	opfun fun;				// instruction function
	int nef_mask;			// ineffectiveness mask
	int nef_result;			// effectiveness result
};

struct iset_instruction {
	uint16_t opcode;		// instruction opcode (and extended opcode)
	uint16_t var_mask;		// variable bits mask
	struct iset_opcode op;	// opcode definition
};

int iset_build(struct iset_opcode **op_tab);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
