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

#include <stdio.h>
#include <arpa/inet.h>
#include "em400_debuger.h"
#include "mjc400.h"
#include "em400_mem.h"
#include "mjc400_regs.h"
#include "mjc400_iset.h"
#include "mjc400_instr.h"
#include "mjc400_timer.h"

// -----------------------------------------------------------------------
void mjc400_reset()
{
	IC = 0;
	IR = 0;
	SR = 0;
	for (int i=0 ; i<16 ; i++) {
		R[i] = 0;
	}
	RZ = 0;
	P = 0;
	MOD = 0;
	MODcnt = 0;
	KB = 0;
}

// -----------------------------------------------------------------------
uint16_t mjc400_fetch_instr()
{
	IC += 1;
	if (SR_Q == 0) {
		return mjc400_os_mem[IC-1];
	} else {
		return mjc400_user_mem[SR_NB][IC-1];
	}
}

// -----------------------------------------------------------------------
int16_t mjc400_fetch_data()
{
	IC += 1;
	return MEM(IC-1);
}

// -----------------------------------------------------------------------
int16_t mjc400_get_eff_arg()
{
	int16_t N = 0;

	// argument is in next word
	if (IR_C == 0) {
		N += mjc400_fetch_data();
	// argument is in field C
	} else {
		N += R[IR_C];
	}

	// add B (if >0, adding 0 won't hurt)
	N += R[IR_B];

	// pre-modification
	N += MOD;
	
	// if D is set, N is an address in current memory block
	if (IR_D == 1) {
		N = MEM(N);
	}
	return N;
}

// -----------------------------------------------------------------------
void mjc400_step()
{
	// do not branch by default
	P = 0;

	// fetch instruction into IR
	// (additional argument is fetched by the instruction, if necessary)
	IR = mjc400_fetch_instr();

	// execute instruction
	int op_res;
	op_res = mjc400_iset[IR_OP].op_fun();

	switch (op_res) {
		// normal instruction
		case OP_OK:
			MOD = 0;
			MODcnt = 0;
			break;
		// pre-modification
		case OP_MD:
			break;
		// illegal instruction
		case OP_ILLEGAL:
			MOD = MODcnt = 0;
			if (P!=0) {
				P = 0;
			} else {
				RZ_6sb;
			}
			break;
	}

	IC += P;
}

// vim: tabstop=4
