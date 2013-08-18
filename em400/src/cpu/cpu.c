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

int16_t N;

// -----------------------------------------------------------------------
void cpu_no_sint()
{
	struct opdef *op = iset_73;
	int deactivated = 0;
	while (deactivated != 2) {
		if ((op->opcode == 0b1010100) || (op->opcode == 0b0010100)) {
			op->op_fun = NULL;
			deactivated++;
		}
		op++;
	}
}

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
void cpu_step()
{
	int _N = 0xbeef;
	struct opdef *op;
	void (*op_fun)();

	// fetch instruction
	regs[R_IR] = nMEM(nR(R_IC));
	LOG(D_CPU, 10, "---- Cycle: Q:NB = %d:%d, IC = 0x%04x ------------", SR_Q, SR_NB, regs[R_IC]);
	regs[R_IC]++;

	op = iset+IR_OP;
	op_fun = op->op_fun;
	if (op->e_opdef) {
		op = op->e_opdef;
	}

	// fetch M-arg if present
	if (op->norm_arg) {
		if (!IR_C) {
			_N = (int16_t) nMEM(nR(R_IC));
			LOG(D_CPU, 20, "Fetched M argument: 0x%04x", _N);
			regs[R_IC]++;
		} else {
			_N = (int16_t) R(IR_C);
		}
	}

	// previous instruction set P?
	if (P) {
		LOG(D_CPU, 20, "P set, skipping");
		P = 0;
		return;
	}

	int op_is_md = (nR(R_IR) & 0b1111110101000000) == 0b1111110101000000 ? 1 : 0;

	// op ineffective?
	if ((op_fun == NULL)
	|| (op_is_md && (regs[R_MODc] >= 3))
	|| (SR_Q && op->user_illegal)) {
		LOG(D_CPU, 10, "Instruction ineffective");
		regs[R_MODc] = 0;
		regs[R_MOD] = 0;
		P = 0;
		int_set(INT_ILLEGAL_OPCODE);
		return;
	}

	P = 0;

	// calculate argument
	if (op->norm_arg) {
		if (IR_B) _N += (int16_t) R(IR_B);
		_N += (int16_t) regs[R_MOD];
		if (IR_D) _N = (int16_t) MEM(_N);
		if (em400_cfg.cpu.mod_17bit) nRw(R_ZC17, (_N >> 16) & 1);
	} else if (op->short_arg) {
		_N = IR_T + (int16_t) regs[R_MOD];
		if (em400_cfg.cpu.mod_17bit) nRw(R_ZC17, (_N >> 16) & 1);
	}
	N = _N;

	LOG(D_CPU, 20, "N/T arg: 0x%04x (%i)", N, N);

	// execute instruction
	LOG(D_CPU, 30, "Execute instruction");
	op_fun();

	if (!op_is_md) {
		regs[R_MODc] = 0;
		regs[R_MOD] = 0;
	}
}

// vim: tabstop=4 shiftwidth=4 autoindent
