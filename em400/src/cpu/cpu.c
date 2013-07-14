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

#include "cpu/cpu.h"
#include "cpu/registers.h"
#include "cpu/interrupts.h"
#include "cpu/memory.h"
#include "cpu/iset.h"
#include "cpu/instructions.h"
#include "cpu/interrupts.h"
#include "cpu/timer.h"

#include "cfg.h"
#include "utils.h"

#ifdef WITH_DEBUGGER
#include "debugger/debugger.h"
#include "debugger/ui.h"
#include "debugger/dasm.h"
#endif
#include "debugger/log.h"

uint16_t M_arg;

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

	// argument is rC or M_arg
	N = IR_C ? R(IR_C) : M_arg;

	// B-modification
	if (IR_B) {
		N += R(IR_B);
	}

	// PRE-modification
	N += nR(R_MOD);
	
	// if D is set, N is an address in current memory block
	if (IR_D) {
		N = MEM((uint16_t) N);
	}

	// store 17th bit for byte addressing
	if (em400_cfg.cpu.mod_17bit) {
		nRw(R_ZC17, (N >> 16) & 1);
	}

	LOG(D_CPU, 10, "------ Effective argument (norm): 0x%04x (%s%s%s%s)", N, IR_C ? "M" : "rC ", regs[R_MODc] ? "PRE-mod" : "", IR_B ? " B-mod" : "", IR_D ? " D-mod" : "");
	return N;
}
#endif

// -----------------------------------------------------------------------
void cpu_step()
{
	struct opdef *op;
	void (*op_fun)();

	// fetch instruction
	nRw(R_IR, nMEM(nR(R_IC)));
	LOG(D_CPU, 3, "---- Cycle: Q:NB = %d:%d, IC = 0x%04x ------------", SR_Q, SR_NB, regs[R_IC]);
	nRinc(R_IC);

	op = iset+IR_OP;
	op_fun = op->op_fun;
	if (op->e_opdef) {
		op = op->e_opdef;
	}

	// fetch M-arg if present
	if (op->m_arg && !IR_C) {
		M_arg = nMEM(nR(R_IC));
		LOG(D_CPU, 3, "Fetched M argument: 0x%04x", M_arg);
		nRinc(R_IC);
	}

	// previous instruction set P?
	if (nR(R_P)) {
		LOG(D_CPU, 3, "P set, skipping");
		Rw(R_P, 0);
		return;
	}

	int op_is_mod = (nR(R_IR) & 0b1111110101000000) == 0b1111110101000000 ? 1 : 0;

	// op ineffective?
	if ((op_fun == NULL)
	|| (op_is_mod && (R(R_MODc) >= 3))
	|| (SR_Q && op->user_illegal)) {
		LOG(D_CPU, 3, "Instruction ineffective");
		Rw(R_MODc, 0);
		Rw(R_MOD, 0);
		Rw(R_P, 0);
		int_set(INT_ILLEGAL_OPCODE);
		return;
	}

	Rw(R_P, 0);

	// execute instruction
	LOG(D_CPU, 3, "Execute instruction");
	op_fun();

	if (!op_mod) {
		nRw(R_MODc, 0);
		nRw(R_MOD, 0);
	}
}

// vim: tabstop=4 shiftwidth=4 autoindent
