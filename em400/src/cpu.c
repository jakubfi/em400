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

#include "cfg.h"
#include "cpu.h"
#include "registers.h"
#include "interrupts.h"
#include "memory.h"
#include "iset.h"
#include "instructions.h"
#include "interrupts.h"
#include "timer.h"
#include "utils.h"

#ifdef WITH_DEBUGGER
#include "debugger/debugger.h"
#include "debugger/ui.h"
#include "debugger/dasm.h"
#endif
#include "debugger/log.h"

#ifdef WITH_SPEEDOPT
uint32_t __N;
#endif

// -----------------------------------------------------------------------
void cpu_reset()
{
	for (int i=0 ; i<R_MAX ; i++) {
		reg_write(i, 0, 0, 1);
	}
	mem_remove_maps();
	int_clear_all();
}

// -----------------------------------------------------------------------
int16_t get_arg_short()
{
	uint32_t T = IR_T + nR(R_MOD);
	if (em400_cfg.cpu.mod_17bit) {
		nRw(R_ZC17, (T >> 16) & 1);
	}
	return T;
}

#ifndef WITH_SPEEDOPT
// -----------------------------------------------------------------------
int16_t get_arg_norm()
{
	uint32_t N;

	LOG(D_CPU, 10, "------ Get argument (norm)");
	// argument is in next word
	if (IR_C == 0) {
		N = nMEM(nR(R_IC));
		nRinc(R_IC);
	// argument is in field C
	} else {
		N = R(IR_C);
	}

	// B-modification
	if (IR_B != 0) {
		N += R(IR_B);
	}

	// PRE-modification
	N += nR(R_MOD);
	
	// if D is set, N is an address in current memory block
	if (IR_D == 1) {
		N = MEM((uint16_t) N);
	}

	// store 17th bit for byte addressing
	if (em400_cfg.cpu.mod_17bit) {
		nRw(R_ZC17, (N >> 16) & 1);
	}

	LOG(D_CPU, 10, "------ Effective argument (norm): 0x%04x (%s%s%s%s)", N, IR_C ? "2-word " : "1-word", regs[R_MODc] ? " PRE-mod" : "", IR_B ? " B-mod" : "", IR_D ? " D-mod" : "");
	return N;
}
#endif

// -----------------------------------------------------------------------
void cpu_step()
{
	LOG(D_CPU, 3, "------ cpu_step() -----------------------------------------------");

	// fetch instruction into IR
	// (additional argument is fetched by the instruction, if necessary)
	nRw(R_IR, nMEM(nR(R_IC)));
	LOG(D_CPU, 3, "Cycle: Q:NB = %d:%d, IC = %d", SR_Q, SR_NB, regs[R_IC]);
	nRinc(R_IC);

#ifdef WITH_DEBUGGER
	char *buf_d;
	char *buf_t;
	char *b;
	uint16_t *addr = mem_ptr(SR_Q * SR_NB, regs[R_IC] - 1);
	int len;
	len = dt_trans(addr, &buf_t, DMODE_TRANS);
	len = dt_trans(addr, &buf_d, DMODE_DASM);
	LOG(D_CPU, 5, "EXEC (words: %i): %s --- %s", len, buf_d, buf_t);
	free(buf_t);
	free(buf_d);
#endif

	static int was_P;

	// execute instruction
	int op_res;
	op_res = iset[IR_OP].op_fun();

	LOG(D_CPU, 3, "------ Check instruction result");

	switch (op_res) {
		// normal instruction
		case OP_OK:
			nRw(R_MOD, 0);
			nRw(R_MODc, 0);
			if (nR(R_P)) {
				nRinc(R_IC);
				Rw(R_P, 0);
				was_P = 1;
			} else {
				was_P = 0;
			}
			break;
		// pre-modification
		case OP_MD:
			was_P = 0;
			break;
		// illegal instruction
		case OP_ILLEGAL:
#ifdef WITH_DEBUGGER
			b = int2bin(nR(IR_OP), 16);
			LOG(D_CPU, 1, "Illegal opcode: %s (0x%04x) at 0x%04x", b, nR(IR_OP), nR(R_IC)-1);
			free(b);
#endif
			nRw(R_MOD, 0);
			nRw(R_MODc, 0);
			if (was_P != 0) {
				was_P = 0;
			} else {
				int_set(INT_ILLEGAL_OPCODE);
			}
			break;
	}

	LOG(D_CPU, 3, "------ End cycle: res = %d, MOD = %d, MODc = %d, P = %d", op_res, regs[R_MOD], regs[R_MODc], regs[R_P]);
}

// vim: tabstop=4
