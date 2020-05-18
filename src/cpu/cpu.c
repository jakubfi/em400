//  Copyright (c) 2012-2018 Jakub Filipowicz <jakubf@gmail.com>
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

#include <emawp.h>

#include "cpu/cpu.h"
#include "cpu/interrupts.h"
#include "mem/mem.h"
#include "cpu/iset.h"
#include "cpu/instructions.h"
#include "cpu/interrupts.h"
#include "cpu/clock.h"
#include "cpu/buzzer.h"
#include "io/io.h"

#include "em400.h"
#include "utils/utils.h"
#include "log.h"
#include "log_crk.h"
#include "ectl/brk.h"

#include "ectl.h" // for global constants
#include "external/iniparser/iniparser.h"

static int cpu_state = ECTL_STATE_OFF;
uint16_t regs[8];
uint16_t rIC;
uint16_t rKB;
int rALARM;
uint16_t rMOD;
int rMODc;
uint16_t rIR;
unsigned RM, Q, BS, NB;

int P;
uint32_t N;
int cpu_mod_active;

struct timespec cpu_timer;
unsigned long ips_counter;
static int speed_real;
static int throttle_granularity;
static float cpu_delay_factor;
pthread_t idler_th;

int cpu_mod_present;
int cpu_user_io_illegal;
static int nomem_stop;

static int sound_enabled;

struct awp *awp;

// opcode table (instruction decoder decision table)
struct iset_opcode *cpu_op_tab[0x10000];

pthread_mutex_t cpu_wake_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cpu_wake_cond = PTHREAD_COND_INITIALIZER;

// -----------------------------------------------------------------------
static void cpu_idle_in_wait()
{
	LOG(L_CPU, "idling in state WAIT");

	pthread_mutex_lock(&cpu_wake_mutex);
	while ((cpu_state == ECTL_STATE_WAIT) && !atom_load_acquire(&RP)) {
			pthread_cond_wait(&cpu_wake_cond, &cpu_wake_mutex);
	}
	cpu_state &= ~ECTL_STATE_WAIT;
	pthread_mutex_unlock(&cpu_wake_mutex);
}

// -----------------------------------------------------------------------
static int cpu_idle_in_stop()
{
	pthread_mutex_lock(&cpu_wake_mutex);
	while ((cpu_state & (ECTL_STATE_STOP|ECTL_STATE_OFF|ECTL_STATE_CLO|ECTL_STATE_CLM|ECTL_STATE_CYCLE)) == ECTL_STATE_STOP) {
		LOG(L_CPU, "idling in state STOP");
		pthread_cond_wait(&cpu_wake_cond, &cpu_wake_mutex);
	}
	int ret = cpu_state;
	pthread_mutex_unlock(&cpu_wake_mutex);
	return ret;
}

// -----------------------------------------------------------------------
void cpu_trigger_state(int state)
{
	pthread_mutex_lock(&cpu_wake_mutex);
	cpu_state |= state;
	pthread_cond_broadcast(&cpu_wake_cond);
	pthread_mutex_unlock(&cpu_wake_mutex);
}

// -----------------------------------------------------------------------
void cpu_clear_state(int state)
{
	pthread_mutex_lock(&cpu_wake_mutex);
	cpu_state &= ~state;
	pthread_cond_broadcast(&cpu_wake_cond);
	pthread_mutex_unlock(&cpu_wake_mutex);
}

// -----------------------------------------------------------------------
int cpu_state_get()
{
	return atom_load_acquire(&cpu_state);
}

// -----------------------------------------------------------------------
void cpu_trigger_cycle()
{
	pthread_mutex_lock(&cpu_wake_mutex);
	if (cpu_state & ECTL_STATE_STOP) {
		cpu_state |= ECTL_STATE_CYCLE;
		pthread_cond_broadcast(&cpu_wake_cond);
	}
	pthread_mutex_unlock(&cpu_wake_mutex);
}

// -----------------------------------------------------------------------
void cpu_mem_fail(int nb)
{
	int_set(INT_NO_MEM);
	if ((nb == 0) && nomem_stop) {
		rALARM = 1;
		cpu_trigger_state(ECTL_STATE_STOP);
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
int cpu_init(dictionary *cfg)
{
	int res;

	int awp_enabled = iniparser_getboolean(cfg, "cpu:awp", 1);
	if (awp_enabled) {
		awp = awp_init(regs+0, regs+1, regs+2, regs+3);
		if (!awp) return LOGERR("Failed to initialize AWP.");
	}

	rKB = iniparser_getint(cfg, "cpu:kb", 0);

	cpu_mod_present = iniparser_getboolean(cfg, "cpu:modifications", 0);
	cpu_user_io_illegal = iniparser_getboolean(cfg, "cpu:user_io_illegal", 1);
	nomem_stop = iniparser_getboolean(cfg, "cpu:stop_on_nomem", 1);
	speed_real = iniparser_getboolean(cfg, "cpu:speed_real", 0);
	throttle_granularity = 1000 * iniparser_getint(cfg, "cpu:throttle_granularity", 10);
	int cpu_speed_factor = iniparser_getdouble(cfg, "cpu:speed_factor", 1.0f);
	cpu_delay_factor = 1.0f/cpu_speed_factor;

	res = iset_build(cpu_op_tab, cpu_user_io_illegal);
	if (res != E_OK) {
		awp_destroy(awp);
		return LOGERR("Failed to build CPU instruction table.");
	}

	int_update_mask(0);

	// this is checked only at power-on
	if (mem_mega_boot()) {
		LOG(L_CPU, "Bootstrap from MEGA PROM is enabled.");
		rIC = 0xf000;
	} else {
		rIC = 0;
	}

	cpu_mod_off();

	sound_enabled = iniparser_getboolean(cfg, "sound:enabled", 0);

	if (sound_enabled) {
		if ((speed_real == 0) || (cpu_speed_factor < 0.5f) || (cpu_speed_factor > 1.5f)) {
			awp_destroy(awp);
			return LOGERR("EM400 needs to be configured with speed_real=true and 1.5 >= cpu_speed_factor >= 0.5 for the buzzer emulation to work.");
		}

		if (buzzer_init(cfg) != E_OK) {
			awp_destroy(awp);
			return LOGERR("Failed to initialize buzzer.");
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
	awp_destroy(awp);
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
static void cpu_clear(int scope)
{
	cpu_clear_state(scope | ECTL_STATE_WAIT);

	// I/O reset should return when we're sure that I/O won't change CPU state (backlogged interrupts, memory writes, ...)
	io_reset();
	mem_reset();
	cpu_mod_off();

	regs[0] = 0;
	SR_write(0);

	int_update_mask(RM);
	int_clear_all();

	if (scope & ECTL_STATE_CLO) {
		rALARM = 0;
		rMOD = 0;
		rMODc = 0;
		cpu_trigger_state(ECTL_STATE_STOP);
	}

	// call even if logging is disabled - user may enable it later
	// and we still want to know if we're running a known OS
	log_check_os();
	log_reset_process();
	log_intlevel_reset();
	log_syscall_reset();
}

// -----------------------------------------------------------------------
int cpu_ctx_switch(uint16_t arg, uint16_t ic, uint16_t int_mask)
{
	uint16_t sp;

	if (!cpu_mem_get(0, 97, &sp)) return 0;

	LOG(L_CPU, "Store current process ctx [IC: 0x%04x, R0: 0x%04x, SR: 0x%04x, 0x%04x] @ 0x%04x, set new IC: 0x%04x", rIC, regs[0], SR_read(), arg, sp, ic);


	if (!cpu_mem_put(0, sp, rIC)) return 0;
	if (!cpu_mem_put(0, sp+1, regs[0])) return 0;
	if (!cpu_mem_put(0, sp+2, SR_read())) return 0;
	if (!cpu_mem_put(0, sp+3, arg)) return 0;
	if (!cpu_mem_put(0, 97, sp+4)) return 0;

	regs[0] = 0;
	rIC = ic;
	Q = 0;
	RM &= int_mask;

	int_update_mask(RM);

	return 1;
}

// -----------------------------------------------------------------------
void cpu_ctx_restore()
{
	uint16_t sr;
	uint16_t sp;

	if (!cpu_mem_get(0, 97, &sp)) return;
	if (!cpu_mem_get(0, sp-4, &rIC)) return;
	if (!cpu_mem_get(0, sp-3, regs)) return;
	if (!cpu_mem_get(0, sp-2, &sr)) return;
	SR_write(sr);
	int_update_mask(RM);
	if (!cpu_mem_put(0, 97, sp-4)) return;

	LOG(L_CPU, "Loaded process ctx @ 0x%04x: [IC: 0x%04x, R0: 0x%04x, SR: 0x%04x]", sp-4, rIC, regs[0], sr);

	return;
}

// -----------------------------------------------------------------------
static int cpu_do_cycle()
{
	struct iset_opcode *op;
	uint16_t data;
	char opcode[32];
	int instruction_time = 0;

	if (LOG_WANTS(L_CPU)) {
		log_store_cycle_state(SR_read(), rIC);
	}

	// fetch instruction
	if (!cpu_mem_get(QNB, rIC, &rIR)) {
		LOGCPU(L_CPU, "        no mem, instruction fetch");
		goto ineffective_memfail;
	}
	op = cpu_op_tab[rIR];
	rIC++;

	// check instruction effectivness
	if (P || ((regs[0] & op->jmp_nef_mask) != op->jmp_nef_result)) {
		if (LOG_WANTS(L_CPU)) {
			log_log_dasm(0, 0, "skip: ");
		}
		if ((op->flags & OP_FL_ARG_NORM) && !IR_C) rIC++;
		goto ineffective;
	} else if (op->flags & OP_FL_ILLEGAL) {
		int2binf(opcode, "... ... . ... ... ...", rIR, 16);
		LOGCPU(L_CPU, "    illegal: %s (0x%04x)", opcode, rIR);
		int_set(INT_ILLEGAL_INSTRUCTION);
		goto ineffective;
	} else if (Q && (op->flags & OP_FL_USR_ILLEGAL)) {
		if (LOG_WANTS(L_CPU)) {
			log_log_dasm(0, 0, "illegal: ");
		}
		int_set(INT_ILLEGAL_INSTRUCTION);
		goto ineffective;
	}

	// prepare argument
	if ((op->flags & OP_FL_ARG_NORM)) {
		if (IR_C) {
			N = regs[IR_C] + rMOD;
		} else {
			if (!cpu_mem_get(QNB, rIC, &data)) {
				LOGCPU(L_CPU, "    no mem, long arg fetch @ %i:0x%04x", QNB, (uint16_t) rIC);
				goto ineffective_memfail;
			} else {
				N = data + rMOD;
				rIC++;
			}
			instruction_time += TIME_MEM_ARG;
		}
		if (rMODc) {
			instruction_time += TIME_PREMOD;
		}
		if (IR_B) {
			N = (uint16_t) N + regs[IR_B];
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
		if (rMODc) {
			instruction_time += TIME_PREMOD;
		}
	}

	if (LOG_WANTS(L_CPU)) {
		log_log_dasm((op->flags & (OP_FL_ARG_NORM | OP_FL_ARG_SHORT)), N, "");
	}

	if (speed_real) {
		instruction_time += op->time;
		if (op->fun == op_72_shc) instruction_time += IR_t * TIME_SHIFT;
	}

	// execute instruction
	op->fun();

	// clear mod if instruction wasn't md
	if (op->fun != op_77_md) {
		rMODc = rMOD = 0;
	}
	return instruction_time;

ineffective_memfail:
	instruction_time += TIME_NOANS_IF;
ineffective:
	instruction_time += TIME_P;
	P = 0;
	rMOD = 0;
	rMODc = 0;
	return instruction_time;
}

// -----------------------------------------------------------------------
void cpu_loop()
{
	pthread_mutex_lock(&cpu_wake_mutex);
	cpu_state = ECTL_STATE_STOP;
	pthread_mutex_unlock(&cpu_wake_mutex);
	cpu_trigger_state(0);

	int cpu_time;
	int cpu_time_cumulative = 0;

	clock_gettime(CLOCK_MONOTONIC, &cpu_timer);

	while (1) {
		cpu_time = 0;
		int state = atom_load_acquire(&cpu_state);

		// CPU running
		if (state == 0) {
cycle:
			// interrupt
			if (atom_load_acquire(&RP) && !P && !rMODc) {
				int_serve();
				cpu_time = TIME_INT_SERVE;
			// CPU cycle
			} else {
				cpu_time = cpu_do_cycle();
				ips_counter++;

				// breakpoint hit?
				if (ectl_brk_check()) {
					cpu_trigger_state(ECTL_STATE_STOP | ECTL_STATE_BRK);
				}
			}

		// quit emulation
		} else if ((state & ECTL_STATE_OFF)) {
			break;

		// CPU reset
		} else if (state & (ECTL_STATE_CLM | ECTL_STATE_CLO)) {
			cpu_clear(state & (ECTL_STATE_CLM | ECTL_STATE_CLO));

		// CPU stopped
		} else if ((state & ECTL_STATE_STOP)) {
			if (sound_enabled) {
				buzzer_stop();
			}
			int res = cpu_idle_in_stop();
			if (speed_real) {
				if (sound_enabled) {
					buzzer_start();
				}
				clock_gettime(CLOCK_MONOTONIC, &cpu_timer);
			}
			if (res & ECTL_STATE_CYCLE) {
				// CPU cycle triggered while in stop
				cpu_clear_state(ECTL_STATE_CYCLE);
				goto cycle;
			}

		// CPU waiting for an interrupt
		} else if ((state & ECTL_STATE_WAIT)) {
			if (sound_enabled) {
				// busy wait for halt when sound is on, but not for speed_real
				cpu_time = throttle_granularity;
			} else {
				// for speed_real just wait
				cpu_idle_in_wait();
			}
		}

		// realtime and sound management
		if (speed_real && (cpu_time != 0)) {
			cpu_time *= cpu_delay_factor;
			if (sound_enabled) {
				buzzer_update(rIR, cpu_time);
			}
			cpu_time_cumulative += cpu_time;
			if (cpu_time_cumulative >= throttle_granularity) {
				cpu_timer.tv_nsec += cpu_time_cumulative;
				while (cpu_timer.tv_nsec > 1000000000) {
					cpu_timer.tv_nsec -= 1000000000;
					cpu_timer.tv_sec++;
				}
				cpu_time_cumulative = 0;
				while (clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &cpu_timer, NULL) == EINTR);
			}
		}
	}
}

// vim: tabstop=4 shiftwidth=4 autoindent
