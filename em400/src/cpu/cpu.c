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

#include "em400.h"
#include "cfg.h"
#include "utils.h"
#include "errors.h"

#ifdef WITH_DEBUGGER
#include "debugger/debugger.h"
#include "debugger/ui.h"
#include "debugger/dasm.h"
#endif
#include "debugger/log.h"

//uint16_t regs[R_MAX];
int P;
int16_t N;
int cpu_mod;

#ifdef WITH_DEBUGGER
uint16_t cycle_ic;
#endif

// -----------------------------------------------------------------------
int cpu_init()
{
	int res;

	regs[R_KB] = em400_cfg.keys;

	// enable enabling cpu modification, if cpu mod is enabled in configuration
	if (em400_cfg.cpu_mod) {
		res = cpu_mod_enable();
		if (res != E_OK) {
			return res;
		}
	}

	if (pthread_spin_init(&int_ready, 0)) {
		return E_SPIN_INIT;
	}
	pthread_spin_trylock(&int_ready);

	cpu_reset();
	return E_OK;
}

// -----------------------------------------------------------------------
void cpu_shutdown()
{
	pthread_spin_unlock(&int_ready);
	pthread_spin_destroy(&int_ready);
}

// -----------------------------------------------------------------------
int cpu_set_op(struct opdef *op_tab, int opcode, opfun fun)
{
	struct opdef *op = op_tab;
	while (op && (op->opcode != 0b1111111)) {
		if (op->opcode == opcode) {
			op->fun = fun;
			return E_OK;
		}
		op++;
	}
	return E_NO_OPCODE;
}

// -----------------------------------------------------------------------
int cpu_mod_enable()
{
	return cpu_set_op(iset_73, 0b0101000, op_73_cron);
}

// -----------------------------------------------------------------------
int cpu_mod_on()
{
	int res;

	cpu_mod = 1;
	int_timer = INT_EXTRA;
	int_extra = INT_TIMER;

	res = cpu_set_op(iset_73, 0b0010100, op_73_sint);
	if (res != E_OK) {
		return res;
	}
	res = cpu_set_op(iset_73, 0b1010100, op_73_sind);
	if (res != E_OK) {
		return res;
	}
	return E_OK;
}

// -----------------------------------------------------------------------
int cpu_mod_off()
{
	int res;

	cpu_mod = 0;
	int_timer = INT_TIMER;
	int_extra = INT_EXTRA;

	res = cpu_set_op(iset_73, 0b0010100, NULL);
	if (res != E_OK) {
		return res;
	}
	res = cpu_set_op(iset_73, 0b1010100, NULL);
	if (res != E_OK) {
		return res;
	}
	return E_OK;
}

// -----------------------------------------------------------------------
void cpu_reset()
{
	int i;
	for (i=0 ; i<R_MAX ; i++) {
		regs[i] = 0;
	}
	mem_reset();
	int_clear_all();
	cpu_mod_off();
	pthread_spin_trylock(&int_ready);
}

// -----------------------------------------------------------------------
void cpu_step()
{
	int32_t N17;
	struct opdef *op;
	opfun op_fun;
	uint16_t data;

#ifdef WITH_DEBUGGER
	cycle_ic = regs[R_IC];
#endif

	// fetch instruction
	if (!mem_cpu_get(QNB, regs[R_IC], regs+R_IR)) {
		regs[R_MODc] = regs[R_MOD] = 0;
		LOG(L_CPU, 10, "    (no mem)");
		return;
	}
	regs[R_IC]++;

	// find instruction
	op = iset + IR_OP;
	if (op->get_eop) {
		op = op->get_eop();
	}
	op_fun = op->fun;

	// end cycle if P is set
	if (P) {
		LOG(L_CPU, 10, "    (skip)");
		P = 0;
		// skip also M-arg if present
		if (op->norm_arg && !IR_C) {
			regs[R_IC]++;
		}
		return;
	}

	// end cycle if op is ineffective
	if (
	(Q && op->user_illegal)
	|| ((regs[R_MODc] >= 3) && (op_fun == op_77_md))
	|| (!op_fun)
	) {
#ifdef WITH_DEBUGGER
	char buf[256];
	dt_trans(cycle_ic, buf, DMODE_DASM);
	LOG(L_CPU, 10, "    (ineffective) %s Q: %d, MODc=%d (%s%s)", buf, Q, regs[R_MODc], op_fun?"legal":"illegal", op->user_illegal?"":", user illegal");
#endif
		regs[R_MODc] = regs[R_MOD] = 0;
		int_set(INT_ILLEGAL_OPCODE);
		// skip also M-arg if present
		if (op->norm_arg && !IR_C) {
			regs[R_IC]++;
		}
		return;
	}

	// process argument
	if (op->norm_arg) {
		if (IR_C) {
			N17 = (int16_t) regs[IR_C] + (int16_t) regs[R_MOD];
		} else {
			if (!mem_cpu_get(QNB, regs[R_IC], &data)) goto catch_nomem;
			N17 = (int16_t) data + (int16_t) regs[R_MOD];
			regs[R_IC]++;
		}
		if (IR_B) {
			N17 += (int16_t) regs[IR_B];
		}
		if (IR_D) {
			if (!mem_cpu_get(QNB, N17, &data)) goto catch_nomem;
			N17 = data;
		}
		if (cpu_mod) {
			regs[R_ZC17] = (N17 >> 16) & 1;
		}
		N = N17;
	} else if (op->short_arg) {
		N17 = (int16_t) IR_T + (int16_t) regs[R_MOD];
		if (cpu_mod) {
			regs[R_ZC17] = (N17 >> 16) & 1;
		}
		N = N17;
	}

	// execute instruction
#ifdef WITH_DEBUGGER
	char buf[256];
	dt_trans(cycle_ic, buf, DMODE_DASM);
	char mbuf[64];
	if (regs[R_MODc]) {
		sprintf(mbuf, "MOD = 0x%04x = %6i", regs[R_MOD], regs[R_MOD]);
	} else {
		*mbuf = '\0';
	}
	if (op->norm_arg) {
		LOG(L_CPU, 10, "    %-20s N = 0x%04x = %i %s", buf, (uint16_t) N, (int16_t) N, mbuf);
	} else if (op->short_arg) {
		LOG(L_CPU, 10, "    %-20s T = %i  %s", buf, (int16_t) N, mbuf);
	} else {
		LOG(L_CPU, 10, "    %-20s", buf);
	}
#endif
	op_fun();

catch_nomem:

	// clear mod if instruction wasn't md
	if (op_fun != op_77_md) {
		regs[R_MODc] = regs[R_MOD] = 0;
	}
}

// vim: tabstop=4 shiftwidth=4 autoindent
