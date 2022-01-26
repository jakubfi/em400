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

static int cpu_state = ECTL_STATE_OFF;

uint16_t r[8];
uint16_t ic, kb, ir, ar, ac;
bool rALARM;
uint16_t rMOD;
int mc;
unsigned rm, nb;
bool p, q, bs;

uint32_t N;

bool cpu_mod_present;
bool cpu_mod_active;
bool cpu_user_io_illegal;
bool awp_enabled;
static bool nomem_stop;

unsigned long ips_counter;

static int speed_real;
static struct timespec cpu_timer;
static int cpu_time_cumulative;
static int throttle_granularity;
static float cpu_delay_factor;

static int sound_enabled;

// opcode table (instruction decoder decision table)
struct iset_opcode *cpu_op_tab[0x10000];

pthread_mutex_t cpu_wake_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cpu_wake_cond = PTHREAD_COND_INITIALIZER;

// -----------------------------------------------------------------------
static void cpu_do_wait()
{
	LOG(L_CPU, "idling in state WAIT");

	pthread_mutex_lock(&cpu_wake_mutex);
	while ((cpu_state == ECTL_STATE_WAIT) && !(atom_load_acquire(&rp) && !p && !mc)) {
			pthread_cond_wait(&cpu_wake_cond, &cpu_wake_mutex);
	}
	cpu_state &= ~ECTL_STATE_WAIT;
	pthread_mutex_unlock(&cpu_wake_mutex);
}

// -----------------------------------------------------------------------
static int cpu_do_stop()
{
	LOG(L_CPU, "idling in state STOP");

	pthread_mutex_lock(&cpu_wake_mutex);
	while ((cpu_state & (ECTL_STATE_STOP|ECTL_STATE_OFF|ECTL_STATE_CLO|ECTL_STATE_CLM|ECTL_STATE_CYCLE|ECTL_STATE_BIN)) == ECTL_STATE_STOP) {
		pthread_cond_wait(&cpu_wake_cond, &cpu_wake_mutex);
	}
	int res = cpu_state;
	pthread_mutex_unlock(&cpu_wake_mutex);

	return res;
}

// -----------------------------------------------------------------------
int cpu_state_change(int to, int from)
{
	int res = 1;

	pthread_mutex_lock(&cpu_wake_mutex);
	if ((from == ECTL_STATE_ANY) || (cpu_state == from)) {
		cpu_state = to;
		pthread_cond_broadcast(&cpu_wake_cond);
		res = 0;
	}
	pthread_mutex_unlock(&cpu_wake_mutex);

	return res;
}

// -----------------------------------------------------------------------
int cpu_state_get()
{
	return atom_load_acquire(&cpu_state);
}

// -----------------------------------------------------------------------
static void cpu_mem_fail(int nb)
{
	int_set(INT_NO_MEM);
	if ((nb == 0) && nomem_stop) {
		rALARM = 1;
		cpu_state_change(ECTL_STATE_STOP, ECTL_STATE_ANY);
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
	int words;
	if ((words = mem_mget(nb, saddr, dest, count)) != count) {
		cpu_mem_fail(nb);
	}
	return words;
}

// -----------------------------------------------------------------------
int cpu_mem_mput(int nb, uint16_t saddr, uint16_t *src, int count)
{
	int words;
	if ((words = mem_mput(nb, saddr, src, count)) != count) {
		cpu_mem_fail(nb);
	}
	return words;
}

// -----------------------------------------------------------------------
int cpu_mem_get_byte(int nb, uint32_t addr, uint8_t *data)
{
	int shift;
	uint16_t orig = 0;

	if (!cpu_mod_active || (q & bs)) addr &= 0xffff;

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

	if (!cpu_mod_active || (q & bs)) addr &= 0xffff;

	shift = 8 * (~addr & 1);
	addr >>= 1;

	if (!cpu_mem_get(nb, addr, &orig)) return 0;
	orig = (orig & (0b1111111100000000 >> shift)) | (((uint16_t) data) << shift);
	if (!cpu_mem_put(nb, addr, orig)) return 0;

	return 1;
}

// -----------------------------------------------------------------------
int cpu_init(em400_cfg *cfg)
{
	int res;

	awp_enabled = cfg_getbool(cfg, "cpu:awp", CFG_DEFAULT_CPU_AWP);

	kb = cfg_getint(cfg, "cpu:kb", CFG_DEFAULT_CPU_KB);

	cpu_mod_present = cfg_getbool(cfg, "cpu:modifications", CFG_DEFAULT_CPU_MODIFICATIONS);
	cpu_user_io_illegal = cfg_getbool(cfg, "cpu:user_io_illegal", CFG_DEFAULT_CPU_IO_USER_ILLEGAL);
	nomem_stop = cfg_getbool(cfg, "cpu:stop_on_nomem", CFG_DEFAULT_CPU_STOP_ON_NOMEM);
	speed_real = cfg_getbool(cfg, "cpu:speed_real", CFG_DEFAULT_CPU_SPEED_REAL);
	throttle_granularity = 1000 * cfg_getint(cfg, "cpu:throttle_granularity", CFG_DEFAULT_CPU_THROTTLE_GRANULARITY);
	double cpu_speed_factor = cfg_getdouble(cfg, "cpu:speed_factor", CFG_DEFAULT_CPU_SPEED_FACTOR);
	cpu_delay_factor = 1.0f/cpu_speed_factor;

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

	LOG(L_CPU, "CPU initialized. AWP: %s, KB=0x%04x, modifications: %s, user I/O: %s, stop on nomem: %s",
		awp_enabled ? "enabled" : "disabled",
		kb,
		cpu_mod_present ? "present" : "absent",
		cpu_user_io_illegal ? "illegal" : "legal",
		nomem_stop ? "true" : "false");
	LOG(L_CPU, "CPU speed: %s, throttle granularity: %i, speed factor: %.2f",
		speed_real ? "real" : "max",
		throttle_granularity/1000,
		cpu_speed_factor);

	sound_enabled = cfg_getbool(cfg, "sound:enabled", CFG_DEFAULT_SOUND_ENABLED);

	if (sound_enabled) {
		if ((speed_real == 0) || (cpu_speed_factor < 0.1f) || (cpu_speed_factor > 2.0f)) {
			LOGERR("EM400 needs to be configured with speed_real=true and 2.0 >= cpu_speed_factor >= 0.1 for the buzzer emulation to work.");
			LOGERR("Disabling sound.");
			sound_enabled = 0;
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
	cpu_mod_active = 1;
	clock_set_int(INT_EXTRA);

	return E_OK;
}

// -----------------------------------------------------------------------
int cpu_mod_off()
{
	cpu_mod_active = 0;
	clock_set_int(INT_CLOCK);

	return E_OK;
}

// -----------------------------------------------------------------------
static void cpu_do_clear(int scope)
{
	// I/O reset should return when we're sure that I/O won't change CPU state (backlogged interrupts, memory writes, ...)
	io_reset();
	mem_reset();
	cpu_mod_off();

	r[0] = 0;
	SR_WRITE(0);

	int_update_mask(rm);
	int_clear_all();

	if (scope == ECTL_STATE_CLO) {
		rALARM = 0;
		rMOD = 0;
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
int cpu_ctx_switch(uint16_t arg, uint16_t new_ic, uint16_t int_mask)
{
	uint16_t sp;

	if (!cpu_mem_get(0, 97, &sp)) return 0;

	LOG(L_CPU, "Store current process ctx [IC: 0x%04x, R0: 0x%04x, SR: 0x%04x, 0x%04x] @ 0x%04x, set new IC: 0x%04x", ic, r[0], SR_READ(), arg, sp, new_ic);


	if (!cpu_mem_put(0, sp, ic)) return 0;
	if (!cpu_mem_put(0, sp+1, r[0])) return 0;
	if (!cpu_mem_put(0, sp+2, SR_READ())) return 0;
	if (!cpu_mem_put(0, sp+3, arg)) return 0;
	if (!cpu_mem_put(0, 97, sp+4)) return 0;

	r[0] = 0;
	ic = new_ic;
	q = 0;
	rm &= int_mask;

	int_update_mask(rm);

	return 1;
}

// -----------------------------------------------------------------------
void cpu_ctx_restore()
{
	uint16_t sr;
	uint16_t sp;

	if (!cpu_mem_get(0, 97, &sp)) return;
	if (!cpu_mem_get(0, sp-4, &ic)) return;
	if (!cpu_mem_get(0, sp-3, r)) return;
	if (!cpu_mem_get(0, sp-2, &sr)) return;
	SR_WRITE(sr);
	int_update_mask(rm);
	if (!cpu_mem_put(0, 97, sp-4)) return;

	LOG(L_CPU, "Loaded process ctx @ 0x%04x: [IC: 0x%04x, R0: 0x%04x, SR: 0x%04x]", sp-4, ic, r[0], sr);
}

// -----------------------------------------------------------------------
static int cpu_do_bin(int start)
{
	static int words = 0;
	static uint16_t data;
	static uint8_t bdata[3];
	static int cnt = 0;

	if (start) {
		LOG(L_CPU, "Binary load initiated @ 0x%04x", ar);
		words = 0;
		cnt = 0;
		return 0;
	}

	int res = io_dispatch(IO_IN, ic, &data);
	if (res == IO_OK) {
		bdata[cnt] = data & 0xff;
		if ((cnt == 0) && bin_is_end(bdata[cnt])) {
			LOG(L_CPU, "Binary load done, %i words loaded", words);
			return 1;
		} else if (bin_is_valid(bdata[cnt])) {
			cnt++;
			if (cnt >= 3) {
				cnt = 0;
				if (cpu_mem_put(0, ar, bin2word(bdata)) == 1) {
					words++;
					ar++;
				}
			}
		}
	}

	return 0;
}

// -----------------------------------------------------------------------
static int cpu_do_cycle()
{
	struct iset_opcode *op;
	uint16_t data;
	char opcode[32];
	int instruction_time = 0;

	if (LOG_WANTS(L_CPU)) {
		log_store_cycle_state(SR_READ(), ic);
	}

	ips_counter++;

	// fetch instruction
	if (!cpu_mem_get(QNB, ic, &ir)) {
		LOGCPU(L_CPU, "        no mem, instruction fetch");
		goto ineffective_memfail;
	}
	op = cpu_op_tab[ir];
	ic++;

	// check instruction effectivness
	if (p || ((r[0] & op->jmp_nef_mask) != op->jmp_nef_result)) {
		if (LOG_WANTS(L_CPU)) {
			log_log_dasm(0, 0, "skip: ");
		}
		if ((op->flags & OP_FL_ARG_NORM) && !IR_C) ic++;
		goto ineffective;
	} else if (op->flags & OP_FL_ILLEGAL) {
		int2binf(opcode, "... ... . ... ... ...", ir, 16);
		LOGCPU(L_CPU, "    illegal: %s (0x%04x)", opcode, ir);
		int_set(INT_ILLEGAL_INSTRUCTION);
		goto ineffective;
	} else if (q && (op->flags & OP_FL_USR_ILLEGAL)) {
		if (LOG_WANTS(L_CPU)) {
			log_log_dasm(0, 0, "illegal: ");
		}
		int_set(INT_ILLEGAL_INSTRUCTION);
		goto ineffective;
	}

	// prepare argument
	if ((op->flags & OP_FL_ARG_NORM)) {
		if (IR_C) {
			N = r[IR_C] + rMOD;
		} else {
			if (!cpu_mem_get(QNB, ic, &data)) {
				LOGCPU(L_CPU, "    no mem, long arg fetch @ %i:0x%04x", QNB, (uint16_t) ic);
				goto ineffective_memfail;
			} else {
				N = data + rMOD;
				ic++;
			}
			instruction_time += TIME_MEM_ARG;
		}
		if (mc) {
			instruction_time += TIME_PREMOD;
		}
		if (IR_B) {
			N = (uint16_t) N + r[IR_B];
			instruction_time += TIME_BMOD;
		}
		if (IR_D) {
			if (!cpu_mem_get(QNB, N, &data)) {
				LOGCPU(L_CPU, "    no mem, indirect arg fetch @ %i:0x%04x", QNB, (uint16_t) N);
				goto ineffective_memfail;
			} else {
				N = data;
			}
			instruction_time += TIME_DMOD;
		}
	} else if ((op->flags & OP_FL_ARG_SHORT)) {
		N = (uint16_t) IR_T + (uint16_t) rMOD;
		if (mc) {
			instruction_time += TIME_PREMOD;
		}
	}

	if (LOG_WANTS(L_CPU)) {
		log_log_dasm((op->flags & (OP_FL_ARG_NORM | OP_FL_ARG_SHORT)), N, "");
	}

	// execute instruction
	op->fun();

	// clear mod if instruction wasn't md
	if (op->fun != op_77_md) {
		mc = rMOD = 0;
	}


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

ineffective_memfail:
	instruction_time += TIME_NOANS_IF;
ineffective:
	instruction_time += TIME_P;
	p = 0;
	rMOD = 0;
	mc = 0;
	return instruction_time;
}

// -----------------------------------------------------------------------
static void cpu_timekeeping(int cpu_time)
{
	int skip_sleep = 0;

	if (cpu_time < 0) {
		cpu_time *= -1;
		skip_sleep = 1;
	}

	cpu_time *= cpu_delay_factor;
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
		cpu_time_cumulative = 0;
		while (clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &cpu_timer, NULL) == EINTR);
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
				cpu_state_change(ECTL_STATE_STOP, ECTL_STATE_CYCLE);
			case ECTL_STATE_RUN:
				if (atom_load_acquire(&rp) && !p && !mc) {
					int_serve();
					cpu_time = TIME_INT_SERVE;
				} else {
					cpu_time = cpu_do_cycle();
					if (ectl_brk_check()) {
						cpu_state_change(ECTL_STATE_STOP, ECTL_STATE_RUN);
					}
				}
				break;
			case ECTL_STATE_OFF:
				if (sound_enabled) buzzer_stop();
				return;
			case ECTL_STATE_CLM:
				cpu_do_clear(ECTL_STATE_CLM);
				cpu_state_change(ECTL_STATE_RUN, ECTL_STATE_CLM);
				break;
			case ECTL_STATE_CLO:
				if (sound_enabled) buzzer_stop();
				cpu_do_clear(ECTL_STATE_CLO);
				cpu_state_change(ECTL_STATE_STOP, ECTL_STATE_CLO);
				break;
			case ECTL_STATE_BIN:
				if (cpu_do_bin(0)) cpu_state_change(ECTL_STATE_STOP, ECTL_STATE_BIN);
				break;
			case ECTL_STATE_STOP:
				if (sound_enabled) buzzer_stop();
				int res = cpu_do_stop();
				if (speed_real && (res == ECTL_STATE_RUN)) {
					if (sound_enabled) buzzer_start();
					clock_gettime(CLOCK_MONOTONIC, &cpu_timer);
					cpu_time_cumulative = 0;
				} else if (res == ECTL_STATE_BIN) {
					cpu_do_bin(1); // initiate binary load
				}
				break;
			case ECTL_STATE_WAIT:
				if (speed_real) {
					if (atom_load_acquire(&rp) && !p && !mc) {
						cpu_state_change(ECTL_STATE_RUN, ECTL_STATE_WAIT);
					} else {
						cpu_time = throttle_granularity;
					}
				}
				else cpu_do_wait();
				break;
		}

		if (speed_real) cpu_timekeeping(cpu_time);
	}
}

// vim: tabstop=4 shiftwidth=4 autoindent
