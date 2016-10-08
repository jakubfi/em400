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

#include <emawp.h>

#include "cpu/cpu.h"
#include "cpu/interrupts.h"
#include "mem/mem.h"
#include "cpu/iset.h"
#include "cpu/instructions.h"
#include "cpu/interrupts.h"
#include "cpu/timer.h"
#include "io/io.h"

#include "em400.h"
#include "cfg.h"
#include "utils.h"
#include "errors.h"
#include "log.h"
#include "log_crk.h"

#ifdef WITH_DEBUGGER
#include "debugger/debugger.h"
#include "debugger/ui.h"
#endif

int cpu_state = STATE_START;

uint16_t regs[8];
uint16_t rIC;
uint16_t rKB;
int rALARM;
uint16_t rMOD;
int rMODc;
uint16_t rIR;
uint16_t rSR;

int P;
uint32_t N;
int cpu_mod_active;

int cpu_mod_present;
int cpu_user_io_illegal;
int exit_on_hlt;
static int nomem_stop;

struct awp *awp;

// -----------------------------------------------------------------------
void cpu_mem_fail(int nb)
{
	int_set(INT_NO_MEM);
	if ((nb == 0) && nomem_stop) {
		rALARM = 1;
		atom_store_release(&cpu_state, STATE_STOP);
	}
}

// -----------------------------------------------------------------------
int cpu_mem_get(int nb, uint16_t addr, uint16_t *data)
{
	if (!mem_get(nb, addr, data)) {
		cpu_mem_fail(nb);
		return 0;
	}
	return 1;
}

// -----------------------------------------------------------------------
int cpu_mem_put(int nb, uint16_t addr, uint16_t data)
{
	if (!mem_put(nb, addr, data)) {
		cpu_mem_fail(nb);
		return 0;
	}
	return 1;
}

// -----------------------------------------------------------------------
int cpu_mem_mget(int nb, uint16_t saddr, uint16_t *dest, int count)
{
	if (!mem_mget(nb, saddr, dest, count)) {
		cpu_mem_fail(nb);
		return 0;
	}
	return 1;
}

// -----------------------------------------------------------------------
int cpu_mem_mput(int nb, uint16_t saddr, uint16_t *src, int count)
{
	if (!mem_mput(nb, saddr, src, count)) {
		cpu_mem_fail(nb);
		return 0;
	}
	return 1;
}

// -----------------------------------------------------------------------
int cpu_mem_get_byte(int nb, uint32_t addr, uint8_t *data)
{
	int shift;
	uint16_t orig = 0;

	if (!cpu_mod_active || (Q & BS)) addr &= 0xffff;

	shift = 8 * (~addr & 1);
	addr >>= 1;

	if (!cpu_mem_get(nb, addr, &orig)) return 0;
	*data = orig >> shift;

	return 1;
}

// -----------------------------------------------------------------------
int cpu_mem_put_byte(int nb, uint32_t addr, uint8_t data)
{
	int shift;
	uint16_t orig = 0;

	if (!cpu_mod_active || (Q & BS)) addr &= 0xffff;

	shift = 8 * (~addr & 1);
	addr >>= 1;

	if (!cpu_mem_get(nb, addr, &orig)) return 0;
	orig = (orig & (0b1111111100000000 >> shift)) | (((uint16_t) data) << shift);
	if (!cpu_mem_put(nb, addr, orig)) return 0;

	return 1;
}

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
int cpu_init(struct cfg_em400 *cfg)
{
	int res;

	if (cfg->cpu_awp) {
		awp = awp_init(regs+0, regs+1, regs+2, regs+3);
		if (!awp) return E_AWP;
	}

	rKB = cfg->keys;

	cpu_mod_present = cfg->cpu_mod;
	cpu_user_io_illegal = cfg->cpu_user_io_illegal;
	exit_on_hlt = cfg->exit_on_hlt;
	nomem_stop = cfg->cpu_stop_on_nomem;

	struct em400_instr *instr = em400_ilist;
	while (instr->var_mask) {
		res = cpu_register_op(em400_op_tab, instr->opcode, instr->var_mask, &instr->op);
		if (res != E_OK) {
			return res;
		}
		instr++;
	}

	int_update_mask(0);

	// seems to be checked only at power-on!
	// (unless power supply sends -PON at hw reset)
	if (mem_mega_boot()) {
		LOG(L_CPU, 1, "Bootstrap from MEGA PROM is enabled");
		rIC = 0xf000;
	}

	cpu_mod_off();

	return E_OK;
}

// -----------------------------------------------------------------------
void cpu_shutdown()
{
	awp_destroy(awp);
}

// -----------------------------------------------------------------------
int cpu_mod_on()
{
	// indicate that CPU modifications are preset
	cpu_mod_active = 1;
	timer_set_int(INT_EXTRA);

	return E_OK;
}

// -----------------------------------------------------------------------
int cpu_mod_off()
{
	// indicate that CPU modifications are absent
	cpu_mod_active = 0;
	timer_set_int(INT_TIMER);

	return E_OK;
}

// -----------------------------------------------------------------------
// software (MCL) and hardware (CP 'CLEAR') reset
void cpu_reset(int hw)
{
	regs[0] = 0;
	rSR = 0;

	if (hw) {
		for (int i=1 ; i<8 ; i++) {
			regs[i] = 0;
		}
		rIC = 0;
		rKB = 0;
		rALARM = 0;
		rMOD = 0;
		rMODc = 0;
		rIR = 0;
	}

	int_update_mask(0);
	int_clear_all();
	cpu_mod_off();

	mem_reset();

	// TODO: move this before CPU clear routine
	// I/O reset should return when we're sure that I/O won't change CPU state (backlogged interrupts, memory writes, ...)
	// this needs MX reset interrupt to become async
	io_reset();

	// TODO: state = STOP, WAIT=0, jakieś inne rejestry?

	// call even if logging is disabled - user may enable it later
	// and we still want to know if we're running a known OS
	log_check_os();
	log_reset_process();
	log_intlevel_reset();
	log_syscall_reset();
}

// -----------------------------------------------------------------------
int cpu_ctx_switch(uint16_t arg, uint16_t ic, uint16_t sr_mask)
{
	uint16_t sp;

	if (!cpu_mem_get(0, 97, &sp)) return 0;

	LOG(L_CPU, 3, "H/W stack push @ 0x%04x (IC = 0x%04x, R0 = 0x%04x, SR = 0x%04x, arg = 0x%04x), new IC = 0x%04x",
		sp,
		rIC,
		regs[0],
		rSR,
		arg,
		ic
	);

	if (!cpu_mem_put(0, sp, rIC)) return 0;
	if (!cpu_mem_put(0, sp+1, regs[0])) return 0;
	if (!cpu_mem_put(0, sp+2, rSR)) return 0;
	if (!cpu_mem_put(0, sp+3, arg)) return 0;
	if (!cpu_mem_put(0, 97, sp+4)) return 0;

	regs[0] = 0;
	rIC = ic;
	rSR &= sr_mask;

	int_update_mask(rSR);

	return 1;
}

// -----------------------------------------------------------------------
int cpu_ctx_restore()
{
	uint16_t data;
	uint16_t sp;

	if (!cpu_mem_get(0, 97, &sp)) return 0;
	if (!cpu_mem_get(0, sp-4, &data)) return 0;
	rIC = data;
	if (!cpu_mem_get(0, sp-3, &data)) return 0;
	regs[0] = data;
	if (!cpu_mem_get(0, sp-2, &data)) return 0;
	rSR = data;
	int_update_mask(rSR);
	if (!cpu_mem_put(0, 97, sp-4)) return 0;

	LOG(L_CPU, 3, "H/W stack pop @ 0x%04x (IC = 0x%04x, R0 = 0x%04x, SR = 0x%04x)",
		sp-4,
		rIC,
		regs[0],
		rSR
	);

	return 1;
}

// -----------------------------------------------------------------------
static void cpu_step()
{
	struct em400_op *op;
	uint16_t data;

	if (LOG_ENABLED) {
		log_store_cycle_state(rSR, rIC);
	}

	// fetch instruction
	if (!mem_get(QNB, rIC, &rIR)) {
		LOGCPU(L_CPU, 2, "        (NO MEM: instruction fetch)");
		goto memfail;
	}

	rIC++;

	// find instruction (by design op is always set to something,
	// even for illegal instructions)
	op = em400_op_tab[rIR];

	// end cycle if P is set
	if (P) {
		LOGCPU(L_CPU, 2, "    (skip)");
		P = 0;
		// skip also M-arg if present
		if (op->norm_arg && !IR_C) rIC++;
		return;
	}

	// process argument
	if (op->norm_arg) {
		if (IR_C) {
			N = regs[IR_C] + rMOD;
		} else {
			if (!mem_get(QNB, rIC, &data)) {
				LOGCPU(L_CPU, 2, "    (no mem: long arg fetch)");
				goto memfail;
			} else {
				N = data + rMOD;
				rIC++;
			}
		}
		if (IR_B) {
			N = (uint16_t) N + regs[IR_B];
		}
		if (IR_D) {
			if (!mem_get(QNB, N, &data)) {
				LOGCPU(L_CPU, 2, "    (no mem: indirect arg fetch)");
				goto memfail;
			} else {
				N = data;
			}
		}
	} else if (op->short_arg) {
		N = (uint16_t) IR_T + (uint16_t) rMOD;
	}

	if (LOG_WANTS(L_CPU, 2)) {
		log_log_dasm(L_CPU, 2, rMOD, op->norm_arg, op->short_arg, N);
	}

	// execute instruction
	op->fun();

	// clear mod if instruction wasn't md
	if (op->fun != op_77_md) {
		rMODc = rMOD = 0;
	}
	return;

memfail:
	cpu_mem_fail(QNB);
	rMODc = rMOD = 0;
}

// -----------------------------------------------------------------------
unsigned int cpu_loop(int autotest)
{
	unsigned long ips_counter = 0;

	while (1) {

		switch (atom_load_acquire(&cpu_state)) {
			case STATE_QUIT:
				return ips_counter;
			case STATE_HALT:
				int_wait();
				atom_store_release(&cpu_state, STATE_START);
				break;
			case STATE_STOP:
#ifdef WITH_DEBUGGER
				dbg_enter = 1;
#endif
				break;
			default:
				break;
		}

		if (atom_load_acquire(&RP) && !P && !rMODc) {
			int_serve();
		}

#ifdef WITH_DEBUGGER
		if (autotest != 1) {
			dbg_step();
		}
#endif

		cpu_step();
		ips_counter++;

	}

	return ips_counter;
}


// vim: tabstop=4 shiftwidth=4 autoindent
