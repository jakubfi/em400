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

#include "cpu.h"
#include "registers.h"
#include "interrupts.h"
#include "memory.h"
#include "iset.h"
#include "instructions.h"
#include "interrupts.h"
#include "timer.h"

#ifdef WITH_DEBUGGER
#include "debugger/debugger.h"
#include "debugger/ui.h"
#include "debugger/dasm.h"
#endif
#include "debugger/log.h"

// -----------------------------------------------------------------------
void cpu_reset()
{
	for (int i=0 ; i<R_MAX ; i++) {
		nRw(i, 0);
	}
	int_clear_all();
}

// -----------------------------------------------------------------------
int16_t get_arg_short()
{
	uint32_t T = IR_T + nR(R_MOD);
	nRw(R_ZC17, (T >> 16) & 1);
	return (int16_t) T;
}

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
	nRw(R_ZC17, (N >> 16) & 1);

	LOG(D_CPU, 10, "------ Effective argument (norm): 0x%04x (%s%s%s%s)", N, IR_C ? "2-word " : "1-word", regs[R_MODc] ? " PRE-mod" : "", IR_B ? " B-mod" : "", IR_D ? " D-mod" : "");
	return (int16_t) N;
}

// -----------------------------------------------------------------------
void cpu_step()
{
	LOG(D_CPU, 1, "------ cpu_step() -----------------------------------------------");

	// fetch instruction into IR
	// (additional argument is fetched by the instruction, if necessary)
	nRw(R_IR, nMEM(nR(R_IC)));
	LOG(D_CPU, 1, "Cycle: Q:NB = %d:%d, IC = %d", SR_Q, SR_NB, regs[R_IC]);
	nRinc(R_IC);

#ifdef WITH_DEBUGGER
	char *buf_d;
	char *buf_t;
	uint16_t *addr = mem_ptr(SR_Q * SR_NB, regs[R_IC] - 1);
	int len;
	len = dt_trans(addr, &buf_t, DMODE_TRANS);
	len = dt_trans(addr, &buf_d, DMODE_DASM);
	LOG(D_CPU, 5, "EXEC: %s --- %s", buf_d, buf_t);
	free(buf_t);
	free(buf_d);
#endif

	// execute instruction
	int op_res;
	op_res = iset[IR_OP].op_fun();

	LOG(D_CPU, 1, "------ Check instruction result");

	switch (op_res) {
		// normal instruction
		case OP_OK:
			nRw(R_MOD, 0);
			nRw(R_MODc, 0);
			break;
		// pre-modification
		case OP_MD:
			break;
		// illegal instruction
		case OP_ILLEGAL:
			nRw(R_MOD, 0);
			nRw(R_MODc, 0);
			int_set(INT_ILLEGAL_OPCODE);
			break;
	}

	if (nR(R_P)) {
		nRinc(R_IC);
		Rw(R_P, 0);
	}

	LOG(D_CPU, 1, "------ End cycle: res = %d, MOD = %d, MODc = %d, P = %d", op_res, regs[R_MOD], regs[R_MODc], regs[R_P]);
}

// vim: tabstop=4
