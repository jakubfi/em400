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

#include "cpu.h"
#include "registers.h"
#include "interrupts.h"
#include "memory.h"
#include "iset.h"
#include "instructions.h"
#include "timer.h"

#ifdef WITH_DEBUGGER
#include "debugger.h"
#include "debugger_ui.h"
#endif

// -----------------------------------------------------------------------
void mjc400_reset()
{
	for (int i=0 ; i<R_MAX ; i++) {
		Rw(i, 0);
#ifdef WITH_DEBUGGER
		reg_act[i] = C_DATA;
#endif
	}
	RZ = 0;
}

// -----------------------------------------------------------------------
int16_t mjc400_get_eff_arg()
{
	uint32_t N;

	// argument is in next word
	if (IR_C == 0) {
		N = MEM(R(R_IC));
		Rinc(R_IC);
	// argument is in field C
	} else {
		N = R(IR_C);
	}

	// B-modification
	N += R(IR_B);

	// PRE-modification
	N += R(R_MOD);
	
	// if D is set, N is an address in current memory block
	if (IR_D == 1) {
		N = MEM((uint16_t) N);
	}

	// store 17th bit for byte addressing
	Rw(R_ZC17, (N & 0b10000000000000000) >> 16);

	return (int16_t) N;
}

// -----------------------------------------------------------------------
void mjc400_step()
{
	// do not branch by default
	Rw(R_P, 0);

	// fetch instruction into IR
	// (additional argument is fetched by the instruction, if necessary)
	Rw(R_IR, MEM(R(R_IC)));
	Rinc(R_IC);

	// execute instruction
	int op_res;
	op_res = mjc400_iset[IR_OP].op_fun();

	switch (op_res) {
		// normal instruction
		case OP_OK:
			Rw(R_MOD, 0);
			Rw(R_MODc, 0);
			break;
		// pre-modification
		case OP_MD:
			break;
		// illegal instruction
		case OP_ILLEGAL:
			Rw(R_MOD, 0);
			Rw(R_MODc, 0);
			if (R(R_P) != 0) {
				Rw(R_P, 0);
			} else {
				INT_SET(INT_ILLEGAL_OPCODE);
			}
			break;
	}

	Radd(R_IC, R(R_P));
}

// vim: tabstop=4
