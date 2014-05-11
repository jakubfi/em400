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
#include "mem/mem.h"
#include "cpu/iset.h"
#include "cpu/instructions.h"
#include "cpu/interrupts.h"
#include "cpu/timer.h"

#include "em400.h"
#include "cfg.h"
#include "utils.h"
#include "errors.h"

#include "dasm/dasm.h"

#ifdef WITH_DEBUGGER
#include "debugger/debugger.h"
#include "debugger/ui.h"
#endif
#include "debugger/log.h"

//uint16_t regs[R_MAX];
int P;
uint32_t N;
int cpu_mod;
uint16_t cycle_ic;

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

	// set IN/OU operations user-legalness
	iset[035].user_illegal = em400_cfg.cpu_user_io_illegal;
	iset[036].user_illegal = em400_cfg.cpu_user_io_illegal;

	// init interrupts
	if (sem_init(&int_ready, 0, 0)) {
		return E_SPIN_INIT;
	}
	int_update_mask(regs[R_SR]);

	cpu_reset();
	return E_OK;
}

// -----------------------------------------------------------------------
void cpu_shutdown()
{
	sem_destroy(&int_ready);
}

// -----------------------------------------------------------------------
int cpu_op_73_set(int opcode, opfun fun)
{
	struct opdef *op = iset_73;
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
	return cpu_op_73_set(0b0101000, op_73_cron);
}

// -----------------------------------------------------------------------
int cpu_mod_on()
{
	int res;

	cpu_mod = 1;
	int_timer = INT_EXTRA;
	int_extra = INT_TIMER;

	res = cpu_op_73_set(0b0010100, op_73_sint);
	if (res != E_OK) {
		return res;
	}
	res = cpu_op_73_set(0b1010100, op_73_sind);
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

	res = cpu_op_73_set(0b0010100, NULL);
	if (res != E_OK) {
		return res;
	}
	res = cpu_op_73_set(0b1010100, NULL);
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
	LOG(L_CPU, 20, "ctx switch: 0x%04x", ic);
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
	LOG(L_CPU, 20, "ctx restore: 0x%04x", data);
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
	struct opdef *op;
	opfun op_fun;
	uint16_t data;

	cycle_ic = regs[R_IC];

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
#if defined(HAVE_DASM)
		char buf[256];
		dt_trans(cycle_ic, buf, DMODE_DASM);
#else
		char buf[256] = "*** undecoded ***";
#endif
		LOG(L_CPU, 10, "    (ineffective) %s Q: %d, MODc=%d (%s%s)", buf, Q, regs[R_MODc], op_fun?"legal":"illegal", op->user_illegal?"":", user illegal");
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
			N = (uint16_t) (regs[IR_C] + regs[R_MOD]);
		} else {
			if (!mem_cpu_get(QNB, regs[R_IC], &data)) goto catch_nomem;
			N = (uint16_t) (data + regs[R_MOD]);
			regs[R_IC]++;
		}
		if (IR_B) {
			N += regs[IR_B];
		}
		if (IR_D) {
			if (!mem_cpu_get(QNB, N, &data)) goto catch_nomem;
			N = data;
		}
	} else if (op->short_arg) {
		N = (uint16_t) IR_T + (uint16_t) regs[R_MOD];
	}

#if defined(WITH_DEBUGGER)
#if defined(HAVE_DASM)
	char buf[256];
	dt_trans(cycle_ic, buf, DMODE_DASM);
#else
	char buf[256] = "*** undecoded ***";
#endif
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

	// execute instruction
	op_fun();

catch_nomem:
	// clear mod if instruction wasn't md
	if (op_fun != op_77_md) {
		regs[R_MODc] = regs[R_MOD] = 0;
	}
}

// vim: tabstop=4 shiftwidth=4 autoindent
