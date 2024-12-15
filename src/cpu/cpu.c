//  Copyright (c) 2012-2021 Jakub Filipowicz <jakubf@gmail.com>
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

#define _XOPEN_SOURCE 600
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <stdbool.h>
#include <emawp.h>

#include "cpu/cpu.h"
#include "cpu/interrupts.h"
#include "mem/mem.h"
#include "cpu/iset.h"
#include "cpu/instructions.h"
#include "cpu/interrupts.h"
#include "cpu/clock.h"
#include "cpu/buzzer.h"
#include "io/defs.h"
#include "io/io.h"

#include "em400.h"
#include "utils/utils.h"
#include "log.h"
#include "log_crk.h"
#include "ectl/brk.h"

#include "ectl.h" // for global constants
#include "cfg.h"

static unsigned cpu_state = ECTL_STATE_OFF;

uint16_t r[8];
uint16_t ic, kb, ir, ac, ar, at;
bool rALARM;
int mc;
unsigned rm, nb;
bool p, q, bs;
uint16_t w;
int r_selected;

bool zc17;

bool cpu_mod_present;
bool cpu_mod_active;
bool cpu_user_io_illegal;
bool awp_enabled;
static bool nomem_stop;

unsigned long ips_counter;
static unsigned instruction_time;

static int speed_real;
static struct timespec cpu_timer;
static int cpu_time_cumulative;
static int throttle_granularity;

static int sound_enabled;

// opcode table (instruction decoder decision table)
struct iset_opcode *cpu_op_tab[0x10000];

pthread_mutex_t cpu_wake_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cpu_wake_cond = PTHREAD_COND_INITIALIZER;


// -----------------------------------------------------------------------
void cpu_kb_set(uint16_t val)
{
	LOG(L_CPU, "KB := 0x%04x", val);

	pthread_mutex_lock(&cpu_wake_mutex);
	kb = val;
	cpu_wake_up();
	pthread_mutex_unlock(&cpu_wake_mutex);
}

// -----------------------------------------------------------------------
static void cpu_do_load(int reg_id, uint16_t val)
{
	LOG(L_CPU, "%s := 0x%04x", ectl_reg_names[reg_id], val);

	switch (reg_id) {
		case ECTL_REG_R0:
		case ECTL_REG_R1:
		case ECTL_REG_R2:
		case ECTL_REG_R3:
		case ECTL_REG_R4:
		case ECTL_REG_R5:
		case ECTL_REG_R6:
		case ECTL_REG_R7: r[reg_id] = val; break;
		case ECTL_REG_IC: ic = val; break;
		case ECTL_REG_AC: ac = val; break;
		case ECTL_REG_AR: ar = val ; break;
		case ECTL_REG_IR: ir = val; break;
		case ECTL_REG_SR: SR_WRITE(val); break;
		case ECTL_REG_RZ: int_put_nchan(val); break;
		default: break;
	}
}

// -----------------------------------------------------------------------
void cpu_register_load(int reg_id, uint16_t val)
{
	pthread_mutex_lock(&cpu_wake_mutex);
	if (cpu_state == ECTL_STATE_STOP) {
		cpu_do_load(reg_id, val);
		cpu_wake_up();
	}
	pthread_mutex_unlock(&cpu_wake_mutex);
}

// -----------------------------------------------------------------------
void cpu_reg_select(int reg_id)
{
	LOG(L_CPU, "Select register: %s", ectl_reg_names[reg_id]);
	pthread_mutex_lock(&cpu_wake_mutex);
	r_selected = reg_id;
	pthread_cond_signal(&cpu_wake_cond);
	pthread_mutex_unlock(&cpu_wake_mutex);
}

// -----------------------------------------------------------------------
static inline void cpu_reg_selected_to_w()
{
	LOG(L_CPU, "W := %s", ectl_reg_names[r_selected]);

	switch (r_selected) {
		case ECTL_REG_R0:
		case ECTL_REG_R1:
		case ECTL_REG_R2:
		case ECTL_REG_R3:
		case ECTL_REG_R4:
		case ECTL_REG_R5:
		case ECTL_REG_R6:
		case ECTL_REG_R7: w = r[r_selected]; break;
		case ECTL_REG_IC: w = ic; break;
		case ECTL_REG_AC: w = ac; break;
		case ECTL_REG_AR: w = ar; break;
		case ECTL_REG_IR: w = ir; break;
		case ECTL_REG_SR: w = SR_READ(); break;
		case ECTL_REG_RZ: w = int_get_nchan(); break;
		case ECTL_REG_KB:
		case ECTL_REG_KB2: w = kb; break;
		default: break;
	}
}

// -----------------------------------------------------------------------
static void cpu_do_fetch()
{
	cpu_mem_read_1(false, ar, &w);
	cpu_do_load(r_selected, w);
	ar++;
}

// -----------------------------------------------------------------------
static void cpu_do_store()
{
	cpu_reg_selected_to_w();
	cpu_mem_write_1(false, ar, w);
	ar++;
}

// -----------------------------------------------------------------------
static int cpu_do_wait()
{
	LOG(L_CPU, "CPU idling");

	pthread_mutex_lock(&cpu_wake_mutex);
	while ((cpu_state == ECTL_STATE_STOP) || ((cpu_state == ECTL_STATE_WAIT) && !(atom_load_acquire(&rp) && !p && !mc))) {
		cpu_reg_selected_to_w();
		pthread_cond_wait(&cpu_wake_cond, &cpu_wake_mutex);
	}
	unsigned res = cpu_state;
	pthread_mutex_unlock(&cpu_wake_mutex);

	return res;
}

// -----------------------------------------------------------------------
void cpu_wake_up()
{
	pthread_cond_signal(&cpu_wake_cond);
}

// -----------------------------------------------------------------------
int cpu_state_change(unsigned to, unsigned from)
{
	int res = 1;

	pthread_mutex_lock(&cpu_wake_mutex);
	if ((from == ECTL_STATE_ANY) || (cpu_state == from)) {
		cpu_state = to;
		pthread_cond_signal(&cpu_wake_cond);
		res = 0;
	}
	pthread_mutex_unlock(&cpu_wake_mutex);

	return res;
}

// -----------------------------------------------------------------------
unsigned cpu_state_get()
{
	return atom_load_acquire(&cpu_state);
}

// -----------------------------------------------------------------------
static void cpu_mem_fail(bool barnb)
{
	instruction_time += TIME_NOANS_IF;
	int_set(INT_NO_MEM);
	if (!barnb) {
		rALARM = true;
		if (nomem_stop) cpu_state_change(ECTL_STATE_STOP, ECTL_STATE_ANY);
	}
}

// -----------------------------------------------------------------------
bool cpu_mem_read_1(bool barnb, uint16_t addr, uint16_t *data)
{
	if (!mem_read_1(barnb * nb, addr, data)) {
		log_cpu(L_CPU, "Read segmentation fault @ %i:0x%04x", barnb*nb, addr);
		cpu_mem_fail(barnb);
		return false;
	}
	return true;
}

// -----------------------------------------------------------------------
bool cpu_mem_write_1(bool barnb, uint16_t addr, uint16_t data)
{
	if (!mem_write_1(barnb * nb, addr, data)) {
		log_cpu(L_CPU, "Write segmentation fault @ %i:0x%04x (data: 0x%04x)", barnb*nb, addr, data);
		cpu_mem_fail(barnb);
		return false;
	}
	return true;
}

// -----------------------------------------------------------------------
int cpu_init(em400_cfg *cfg)
{
	int res;

	awp_enabled = cfg_getbool(cfg, "cpu:awp", CFG_DEFAULT_CPU_AWP);

	cpu_mod_present = cfg_getbool(cfg, "cpu:modifications", CFG_DEFAULT_CPU_MODIFICATIONS);
	cpu_user_io_illegal = cfg_getbool(cfg, "cpu:user_io_illegal", CFG_DEFAULT_CPU_IO_USER_ILLEGAL);
	nomem_stop = cfg_getbool(cfg, "cpu:stop_on_nomem", CFG_DEFAULT_CPU_STOP_ON_NOMEM);
	speed_real = cfg_getbool(cfg, "cpu:speed_real", CFG_DEFAULT_CPU_SPEED_REAL);
	throttle_granularity = 1000 * cfg_getint(cfg, "cpu:throttle_granularity", CFG_DEFAULT_CPU_THROTTLE_GRANULARITY);

	res = iset_build(cpu_op_tab, cpu_user_io_illegal);
	if (res != E_OK) {
		return LOGERR("Failed to build CPU instruction table.");
	}

	int_update_mask(0);

	// this is checked only at power-on
	if (mem_mega_boot()) {
		ic = 0xf000;
	} else {
		ic = 0;
	}

	cpu_mod_off();

	LOG(L_CPU, "CPU initialized. AWP: %s, modifications: %s, user I/O: %s, stop on nomem: %s",
		awp_enabled ? "enabled" : "disabled",
		cpu_mod_present ? "present" : "absent",
		cpu_user_io_illegal ? "illegal" : "legal",
		nomem_stop ? "true" : "false");
	LOG(L_CPU, "CPU speed: %s, throttle granularity: %i ns",
		speed_real ? "real" : "unlimited",
		throttle_granularity);

	sound_enabled = cfg_getbool(cfg, "sound:enabled", CFG_DEFAULT_SOUND_ENABLED);

	if (sound_enabled) {
		if (!speed_real) {
			LOGERR("WARNING: sound won't work with speed_real=false. Buzzer emulation is disabled.");
		} else {
			if (buzzer_init(cfg) != E_OK) {
				return LOGERR("Failed to initialize buzzer.");
			}
		}
	}

	return E_OK;
}

// -----------------------------------------------------------------------
void cpu_shutdown()
{
	if (sound_enabled) {
		buzzer_shutdown();
	}
}

// -----------------------------------------------------------------------
int cpu_mod_on()
{
	cpu_mod_active = true;
	clock_set_int(INT_EXTRA);

	return E_OK;
}

// -----------------------------------------------------------------------
int cpu_mod_off()
{
	cpu_mod_active = false;
	clock_set_int(INT_CLOCK);

	return E_OK;
}

// -----------------------------------------------------------------------
void cpu_do_clear(bool clo)
{
	// I/O reset should return when we're sure that I/O won't change CPU state (backlogged interrupts, memory writes, ...)
	io_reset();
	mem_reset(clo); // MEGA memory handles manual (long) reset differently
	cpu_mod_off();

	r[0] = 0;
	SR_WRITE(0);
	int_update_mask(rm);
	int_clear_all();

	if (clo) {
		rALARM = false;
		mc = 0;
	}

	// call even if logging is disabled - user may enable it later
	// and we still want to know if we're running a known OS
	log_check_os();
	log_reset_process();
	log_intlevel_reset();
	log_syscall_reset();
}

// -----------------------------------------------------------------------
void cpu_ctx_switch(uint16_t arg, uint16_t new_ic, uint16_t int_mask)
{
	if (!cpu_mem_read_1(false, STACK_POINTER, &ar)) return;

	LOG(L_CPU, "Store current process ctx [IC: 0x%04x, R0: 0x%04x, SR: 0x%04x, 0x%04x] @ 0x%04x, set new IC: 0x%04x", ic, r[0], SR_READ(), arg, ar, new_ic);

	uint16_t vector[] = { ic, r[0], SR_READ(), arg };
	for (int i=0 ; i<4 ; i++, ar++) {
		if (!cpu_mem_write_1(false, ar, vector[i])) return;
	}
	if (!cpu_mem_write_1(false, STACK_POINTER, ar)) return;

	r[0] = 0;
	ic = new_ic;
	q = false;
	rm &= int_mask;
	int_update_mask(rm);
}

// -----------------------------------------------------------------------
void cpu_sp_rewind()
{
	if (!cpu_mem_read_1(false, STACK_POINTER, &ar)) return;
	ar -= 4;
	if (!cpu_mem_write_1(false, STACK_POINTER, ar)) return;
}

// -----------------------------------------------------------------------
void cpu_ctx_restore(bool barnb)
{
	uint16_t sr_tmp;
	uint16_t *vector[] = { &ic, r+0, &sr_tmp };
	for (int i=0 ; i<3 ; i++, ar++) {
		if (!cpu_mem_read_1(barnb, ar, vector[i])) return;
	}
	SR_WRITE(sr_tmp);
	int_update_mask(rm);
}

// -----------------------------------------------------------------------
static void cpu_do_bin()
{
	int lg = 0;
	uint16_t rb;

	LOG(L_CPU, "Binary load initiated @ 0x%04x", ar);
K1:
	// allow emulation to break free from failed loads with CLEAR
	if (cpu_state_get() == ECTL_STATE_CLO) return;

	io_dispatch(IO_IN, ic, &w);

	// TODO: clear extenral interrupts?
	switch (lg) {
		case 0:
			rb = (w & 0b0000000000001111) << 12;
			break;
		case 1:
			rb |= (w & 0b0000000000111111) << 6;
			break;
		case 2:
			rb |= (w & 0b0000000000111111);
			break;
	}
	// STEP here
	if (((w & BIN_ENDBYTE) == BIN_ENDBYTE) && (lg == 0)) {
		LOG(L_CPU, "Binary load done, AR: 0x%04x", ar);
		return;
	}
	if ((w & 0b0000000001000000)) {
		lg = lg + 1;
	}
	if (lg != 3) goto K1;
	if (lg == 3) goto K2;

K2:
	// small visual feedback during binary load
	// TODO: real I/O devices speeds
	usleep(1);

	w = rb;
	cpu_mem_write_1(q, ar, w);
	ar++;
	lg = 0;
	// STEP here
	goto K1;
}

// -----------------------------------------------------------------------
static int cpu_do_cycle()
{
	struct iset_opcode *op;
	bool PR = true;
	unsigned flags;
	bool xi;

	if (LOG_WANTS(L_CPU)) log_store_cycle_state(SR_READ(), ic);

	ips_counter++;
	instruction_time = 0;

P1:
	cpu_mem_read_1(q, ic, &w);
	ic++;
	if (PR) {
		ir = w;
		PR = false;
	} else {
		ac = w;
		if (!mc) ar = w;
		instruction_time += TIME_MEM_ARG;
		goto P3_P4_P5;
	}

	op = cpu_op_tab[ir];
	flags = op->flags;

	// ineffective instructions
	if (p) goto P2;
	if((r[0] & op->jmp_nef_mask) != op->jmp_nef_result) goto P2;
	// illegal instructions (also ineffective)
	if (flags & OP_FL_ILLEGAL) goto P2;
	if (q && (flags & OP_FL_USR_ILLEGAL)) goto P2;
	if ((op->fun == op_77_md) && (mc == 3)) goto P2;

	// process immediate argument
	if ((flags & OP_FL_ARG_NORM) && (IR_C == 0)) goto P1;

P3_P4_P5:
// P3
	if (flags & OP_FL_ARG_NORM && IR_C) {
		w = r[IR_C];
		ac = w;
		if (!mc) ar = w;
	} else if (flags & OP_FL_ARG_SHORT) {
		w = IR_T;
		ac = w;
		if (!mc) ar = w;
	} else if (flags & OP_FL_ARG_BYTE) {
		w = IR_b;
		ac = w;
		if (!mc) ar = w;
	}

// P4 pre-mod
	if (mc) {
		zc17 = (ac + ar) > 0xffff;
		w = ac + ar;
		ar = ac = w;
		instruction_time += TIME_PREMOD;
	} else {
		mc = 0;
		zc17 = false;
	}

// P4 B-mod
	if ((flags & OP_FL_ARG_NORM) && IR_B) {
		zc17 = (ac + r[IR_B]) > 0xffff;
		w = ac + r[IR_B];
		ar = ac = w;
		instruction_time += TIME_BMOD;
	}

// P5 D-mod
	if ((flags & OP_FL_ARG_NORM) && IR_D) {
		cpu_mem_read_1(q, ar, &w);
		ar = ac = w;
		instruction_time += TIME_DMOD;
	}

	if (op->fun != op_77_md) mc = 0;

	// execute instruction
	log_dasm((op->flags & (OP_FL_ARG_NORM | OP_FL_ARG_SHORT)), ac, "");
	op->fun();
	instruction_time += op->time;

	if (op->fun == op_72_shc) {
		instruction_time += IR_t * TIME_SHIFT;
	} else if (op->fun == op_ou) {
		// Negative instruction time means "skip time keeping for this cycle".
		// Do this after each OU instruction.
		// This is required for minimalistic I/O routines using OU+HLT to work.
		// Without it, it may happen that interrupt HLT was supposed to wait for
		// is served just after OU, causing HLT to sleep indefinitely.
		instruction_time *= -1;
	}

	return instruction_time;

P2: // ineffective and illegal instructions handler
	xi = (flags & OP_FL_ILLEGAL) || (q && (flags & OP_FL_USR_ILLEGAL)) || ((op->fun == op_77_md) && (mc == 3));
	if (xi && !p) {
		// instruction is illegal and skip flag is not set
		log_dasm(0, 0, "ILL: ");
		int_set(INT_ILLEGAL_INSTRUCTION);
	} else {
		// instruction is ineffective, immediate word argument is skipped
		log_dasm(0, 0, "NEF: ");
		if ((flags & OP_FL_ARG_NORM) && !IR_C) ic++;
	}
	instruction_time += TIME_P;
	p = false;
	mc = 0;
	return instruction_time;
}

// -----------------------------------------------------------------------
static void cpu_timekeeping(int cpu_time)
{
	bool skip_sleep = false;

	if (cpu_time < 0) {
		cpu_time *= -1;
		skip_sleep = true;
	}

	cpu_time_cumulative += cpu_time;

	if (sound_enabled) {
		buzzer_update(ir, cpu_time);
	}

	if (!skip_sleep && (cpu_time_cumulative >= throttle_granularity)) {
		cpu_timer.tv_nsec += cpu_time_cumulative;
		while (cpu_timer.tv_nsec >= 1000000000) {
			cpu_timer.tv_nsec -= 1000000000;
			cpu_timer.tv_sec++;
		}
		while (clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &cpu_timer, NULL) == EINTR);
		cpu_time_cumulative = 0;
	}
}

// -----------------------------------------------------------------------
void cpu_loop()
{
	cpu_state_change(ECTL_STATE_STOP, ECTL_STATE_ANY);
	clock_gettime(CLOCK_MONOTONIC, &cpu_timer);

	while (1) {
		int cpu_time = 0;
		int state = atom_load_acquire(&cpu_state);

		switch (state) {
			case ECTL_STATE_CYCLE:
				cpu_state_change(ECTL_STATE_STOP, ECTL_STATE_ANY);
				// fallthrough
			case ECTL_STATE_RUN:
				if (atom_load_acquire(&rp) && !p && (mc == 0)) {
					int_serve();
					cpu_time = TIME_INT_SERVE;
				} else {
					cpu_time = cpu_do_cycle();
					if (ectl_brk_check()) {
						cpu_state_change(ECTL_STATE_STOP, ECTL_STATE_ANY);
					}
				}
				break;
			case ECTL_STATE_OFF:
				if (sound_enabled) buzzer_stop();
				return;
			case ECTL_STATE_CLO:
				if (sound_enabled) buzzer_stop();
				cpu_do_clear(true);
				cpu_state_change(ECTL_STATE_STOP, ECTL_STATE_ANY);
				break;
			case ECTL_STATE_BIN:
				cpu_do_bin();
				cpu_state_change(ECTL_STATE_STOP, ECTL_STATE_BIN);
				break;
			case ECTL_STATE_LOAD:
				w = kb;
				cpu_do_load(r_selected, w);
				cpu_state_change(ECTL_STATE_STOP, ECTL_STATE_LOAD);
				break;
			case ECTL_STATE_STORE:
				cpu_do_store();
				cpu_state_change(ECTL_STATE_STOP, ECTL_STATE_STORE);
				break;
			case ECTL_STATE_FETCH:
				cpu_do_fetch();
				cpu_state_change(ECTL_STATE_STOP, ECTL_STATE_FETCH);
				break;
			case ECTL_STATE_STOP:
				if (sound_enabled) buzzer_stop();
				int res = cpu_do_wait();
				if (res == ECTL_STATE_RUN) {
					if (sound_enabled) buzzer_start();
					clock_gettime(CLOCK_MONOTONIC, &cpu_timer);
					cpu_time_cumulative = 0;
				}
				break;
			case ECTL_STATE_WAIT:
				// busy wait to not disturb audio, TODO
				cpu_reg_selected_to_w();
				if (atom_load_acquire(&rp) && !p && !mc) {
					cpu_state_change(ECTL_STATE_RUN, ECTL_STATE_WAIT);
				} else {
					cpu_time = throttle_granularity;
				}
				// else = if (!speed_real) {
				//	cpu_do_wait();
				//	cpu_state_change(ECTL_STATE_RUN, ECTL_STATE_WAIT);
				//}
				break;
		}

		if (speed_real) cpu_timekeeping(cpu_time);
	}
}

// vim: tabstop=4 shiftwidth=4 autoindent
