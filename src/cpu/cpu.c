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

#include <string.h>

#include "cpu/cpu.h"
#include "cpu/registers.h"
#include "cpu/interrupts.h"
#include "mem/mem.h"
#include "cpu/iset.h"
#include "cpu/instructions.h"
#include "cpu/interrupts.h"
#include "cpu/timer.h"

#include "em400.h"
#include "cfg.h"
#include "utils.h"
#include "errors.h"
#include "emulog.h"

int P;
uint32_t N;
int cpu_mod_active;

// -----------------------------------------------------------------------
static int cpu_register_op(struct em400_op **op_tab, uint16_t opcode, uint16_t mask, struct em400_op *op)
{
	int i, pos;
	int offsets[16];
	int one_count = 0;
	int max;
	uint16_t result;

	// if mask is empty - nothing to do
	if (!mask) return E_OK;

	// store 1's positions in mask, count 1's
	for (i=0 ; i<16 ; i++) {
		if (mask & (1<<i)) {
			offsets[one_count] = i;
			one_count++;
		}
	}

	max = (1 << one_count) - 1;

	// iterate over all variants (as indicated by the mask)
	for (i=0 ; i<=max ; i++) {
		result = 0;
		// shift 1's into positions
		for (pos=one_count-1 ; pos>=0 ; pos--) {
			result |= ((i >> pos) & 1) << offsets[pos];
		}
		// sanity check: we don't want to overwrite non-illegal registered ops
		if ((op_tab[opcode | result]) && (op_tab[opcode | result]->fun != op_illegal)) {
			return E_SLID_INIT;
		}
		// register the op
		op_tab[opcode | result] = op;
	}
	return E_OK;
}

// -----------------------------------------------------------------------
int cpu_init()
{
	int res;

	regs[R_KB] = em400_cfg.keys;

	struct em400_instr *instr = em400_ilist;
	while (instr->var_mask) {
		res = cpu_register_op(em400_op_tab, instr->opcode, instr->var_mask, &instr->op);
		if (res != E_OK) {
			return res;
		}
		instr++;
	}

	cpu_reset();

	int_update_mask(regs[R_SR]);

	if (mem_mega_boot()) {
		eprint("Bootstrap from MEGA PROM is enabled\n");
		regs[R_IC] = 0xf000;
	}

	return E_OK;
}

// -----------------------------------------------------------------------
void cpu_shutdown()
{
}

// -----------------------------------------------------------------------
int cpu_mod_on()
{
	// indicate that CPU modifications are preset
	cpu_mod_active = 1;

	return E_OK;
}

// -----------------------------------------------------------------------
int cpu_mod_off()
{
	// indicate that CPU modifications are absent
	cpu_mod_active = 0;

	return E_OK;
}

// -----------------------------------------------------------------------
void cpu_reset()
{
	// disable cpu modifications
	cpu_mod_off();

	// clear registers
	for (int i=0 ; i<R_MAX ; i++) {
		regs[i] = 0;
	}

	// reset memory configuration
	mem_reset();

	// clear all interrupts
	int_clear_all();
}

// -----------------------------------------------------------------------
int cpu_ctx_switch(uint16_t arg, uint16_t ic, uint16_t sr_mask)
{
	uint16_t sp;
	if (!mem_cpu_get(0, 97, &sp)) return 0;
	if (!mem_cpu_put(0, sp, regs[R_IC])) return 0;
	if (!mem_cpu_put(0, sp+1, regs[0])) return 0;
	if (!mem_cpu_put(0, sp+2, regs[R_SR])) return 0;
	if (!mem_cpu_put(0, sp+3, arg)) return 0;
	if (!mem_cpu_put(0, 97, sp+4)) return 0;
	regs[0] = 0;
	regs[R_IC] = ic;
	regs[R_SR] &= sr_mask;
	int_update_mask();
	EMULOGCPU(L_CPU, 3, "ctx switch, IC = 0x%04x", ic);
	return 1;
}

// -----------------------------------------------------------------------
int cpu_ctx_restore()
{
	uint16_t data;
	uint16_t sp;

	if (!mem_cpu_get(0, 97, &sp)) return 0;
	if (!mem_cpu_get(0, sp-4, &data)) return 0;
	regs[R_IC] = data;
	EMULOGCPU(L_CPU, 3, "ctx restore, IC = 0x%04x", data);
	if (!mem_cpu_get(0, sp-3, &data)) return 0;
	regs[0] = data;
	if (!mem_cpu_get(0, sp-2, &data)) return 0;
	regs[R_SR] = data;
	int_update_mask();
	if (!mem_cpu_put(0, 97, sp-4)) return 0;
	return 1;
}

// -----------------------------------------------------------------------
void cpu_step()
{
	struct em400_op *op;
	uint16_t data;

	if (EMULOG_ENABLED) {
		emulog_store_cycle_state(regs[R_SR], regs[R_IC]);
	}

	// fetch instruction
	if (!mem_cpu_get(QNB, regs[R_IC], regs+R_IR)) {
		regs[R_MODc] = regs[R_MOD] = 0;
		EMULOGCPU(L_CPU, 2, "        (NO MEM: instruction fetch)");
		return;
	}
	regs[R_IC]++;

	// find instruction (by design op is always set to something,
	// even for illegal instructions)
	op = em400_op_tab[regs[R_IR]];

	// end cycle if P is set
	if (P) {
		EMULOGCPU(L_CPU, 2, "    (skip)");
		P = 0;
		// skip also M-arg if present
		if (op->norm_arg && !IR_C) regs[R_IC]++;
		return;
	}

	// process argument
	if (op->norm_arg) {
		if (IR_C) {
			N = regs[IR_C] + regs[R_MOD];
		} else {
			if (!mem_cpu_get(QNB, regs[R_IC], &data)) {
				EMULOGCPU(L_CPU, 2, "    (no mem: long arg fetch)");
				goto finish;
			} else {
				N = data + regs[R_MOD];
				regs[R_IC]++;
			}
		}
		if (IR_B) {
			N = (uint16_t) N + regs[IR_B];
		}
		if (IR_D) {
			if (!mem_cpu_get(QNB, N, &data)) {
				EMULOGCPU(L_CPU, 2, "    (no mem: indirect arg fetch)");
				goto finish;
			} else {
				N = data;
			}
		}
	} else if (op->short_arg) {
		N = (uint16_t) IR_T + (uint16_t) regs[R_MOD];
	}

	if (EMULOG_WANTS(L_CPU, 2)) {
		emulog_log_dasm(L_CPU, 2, regs[R_MOD], op->norm_arg, op->short_arg, N);
	}

	// execute instruction
	op->fun();

	// clear mod if instruction wasn't md
	if (op->fun != op_77_md) {
finish:
		regs[R_MODc] = 0;
		regs[R_MOD] = 0;
	}
}

// vim: tabstop=4 shiftwidth=4 autoindent
