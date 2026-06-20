//  Copyright (c) 2012-2024 Jakub Filipowicz <jakubf@gmail.com>
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
#include <stdatomic.h>
#include <string.h>
#include <pthread.h>
#include <limits.h>

#include <emawp.h>

#include "utils/utils.h"
#include "utils/compat_time.h"
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

#include "libem400.h"
#include "utils/utils.h"
#include "log.h"
#include "log_crk.h"
#include "cp/brk.h"
#include "cp/cp.h"

bool cpu_initialized;

static atomic_uint cpu_state = EM400_STATE_OFF;

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

atomic_ulong ips_counter;
// negative instruction_time_ns = skip next cpu sleep
static int instruction_time_ns;

static int speed_real;
static struct timespec cpu_timer;
static int cpu_time_cumulative_ns;
static int emulation_quantum_ns;
static int ticks_per_2s;

#define TIMING_PROBE_DELAY			10
#define TIMING_PROBE_PASSES			10
#define TIMING_BUSY_THRESHOLD_NS	100000L		// 100 us: BUSY->PROBE gate
#define TIMING_PROBE_THRESHOLD_NS	500000L		// 500 us: PROBE->BUSY pass gate
#define TIMING_SLEEP_THRESHOLD_NS	1000000L	// 1 ms:   SLEEP->BUSY fallback
enum timing_policy_e { TIMING_POLICY_BUSY, TIMING_POLICY_PROBE, TIMING_POLICY_SLEEP, TIMING_POLICY_COUNT };

static int sound_enabled;

// opcode table (instruction decoder decision table)
struct iset_opcode *cpu_op_tab[0x10000];

pthread_t cpu_thread;
pthread_mutex_t cpu_wake_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cpu_wake_cond = PTHREAD_COND_INITIALIZER;

static void * cpu_loop(void *ptr);

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
	LOG(L_CPU, "%s := 0x%04x", em400_reg_name(reg_id), val);

	switch (reg_id) {
		case EM400_REG_R0:
		case EM400_REG_R1:
		case EM400_REG_R2:
		case EM400_REG_R3:
		case EM400_REG_R4:
		case EM400_REG_R5:
		case EM400_REG_R6:
		case EM400_REG_R7: r[reg_id] = val; break;
		case EM400_REG_IC: ic = val; break;
		case EM400_REG_AC: ac = val; break;
		case EM400_REG_AR: ar = val ; break;
		case EM400_REG_IR: ir = val; break;
		case EM400_REG_SR: SR_WRITE(val); int_update_xmask(); break;
		case EM400_REG_RZ: int_put_nchan(val); break;
		default: break;
	}
}

// -----------------------------------------------------------------------
void cpu_reg_load(unsigned reg_id, uint16_t val)
{
	pthread_mutex_lock(&cpu_wake_mutex);
	if (cpu_state_get() == EM400_STATE_STOP) {
		cpu_do_load(reg_id, val);
		cpu_wake_up();
	}
	pthread_mutex_unlock(&cpu_wake_mutex);
}

// -----------------------------------------------------------------------
void cpu_reg_select(unsigned reg_id)
{
	LOG(L_CPU, "Select register: %s", em400_reg_name(reg_id));
	pthread_mutex_lock(&cpu_wake_mutex);
	r_selected = reg_id;
	pthread_cond_signal(&cpu_wake_cond);
	pthread_mutex_unlock(&cpu_wake_mutex);
}

// -----------------------------------------------------------------------
int cpu_reg_fetch(unsigned reg_id)
{
	switch (reg_id) {
		case EM400_REG_R0:
		case EM400_REG_R1:
		case EM400_REG_R2:
		case EM400_REG_R3:
		case EM400_REG_R4:
		case EM400_REG_R5:
		case EM400_REG_R6:
		case EM400_REG_R7: return r[reg_id];
		case EM400_REG_IC: return ic;
		case EM400_REG_AC: return ac;
		case EM400_REG_AR: return ar;
		case EM400_REG_IR: return ir;
		case EM400_REG_SR: return SR_READ();
		case EM400_REG_RZ: return int_get_nchan();
		case EM400_REG_KB:
		case EM400_REG_KB2: return kb;
		default: return -1;
	}
}

// -----------------------------------------------------------------------
static inline void cpu_reg_selected_to_w()
{
	w = cpu_reg_fetch(r_selected);
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
	while ((cpu_state_get() == EM400_STATE_STOP) || ((cpu_state_get() == EM400_STATE_WAIT) && !(atomic_load_explicit(&irq, memory_order_acquire) && !p && !mc))) {
		cpu_reg_selected_to_w();
		pthread_cond_wait(&cpu_wake_cond, &cpu_wake_mutex);
	}
	unsigned res = cpu_state_get();
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
	unsigned last_state = atomic_load_explicit(&cpu_state, memory_order_acquire);
	if ((from == EM400_STATE_ANY) || (last_state == from)) {
		// the last breakpoint hit is valid only while stopped on it
		// RUN/CYCLE/CLO invalidates it
		switch (to) {
			case EM400_STATE_RUN:
			case EM400_STATE_CYCLE:
			case EM400_STATE_CLO:
				brk_hit_clear();
				break;
			default:
				break;
		}
		atomic_store_explicit(&cpu_state, to, memory_order_release);
		pthread_cond_signal(&cpu_wake_cond);
		res = 0;
	}
	pthread_mutex_unlock(&cpu_wake_mutex);

	LOG(L_CPU, "CPU state change (current: %s): %s -> %s%s",
		em400_cpu_state_name(last_state),
		em400_cpu_state_name(from),
		em400_cpu_state_name(to),
		res ? " (ineffective)" : ""
	);

	return res;
}

// -----------------------------------------------------------------------
unsigned cpu_state_get()
{
	return atomic_load_explicit(&cpu_state, memory_order_acquire);
}

// -----------------------------------------------------------------------
static void cpu_mem_fail(bool barnb)
{
	instruction_time_ns += TIME_NOANS_IF;
	int_set(INT_NO_MEM);
	if (!barnb) {
		rALARM = true;
		if (nomem_stop) cpu_state_change(EM400_STATE_STOP, EM400_STATE_ANY);
	}
}

// -----------------------------------------------------------------------
bool cpu_mem_read_1(bool barnb, uint16_t addr, uint16_t *data)
{
	if (!mem_read_1(barnb * nb, addr, data)) {
		LOGCPU(L_CPU, "Read segmentation fault @ %i:0x%04x", barnb*nb, addr);
		cpu_mem_fail(barnb);
		return false;
	}
	return true;
}

// -----------------------------------------------------------------------
bool cpu_mem_write_1(bool barnb, uint16_t addr, uint16_t data)
{
	if (!mem_write_1(barnb * nb, addr, data)) {
		LOGCPU(L_CPU, "Write segmentation fault @ %i:0x%04x (data: 0x%04x)", barnb*nb, addr, data);
		cpu_mem_fail(barnb);
		return false;
	}
	return true;
}

// -----------------------------------------------------------------------
int cpu_init(const struct em400_host_cfg *host, const struct em400_machine_cfg *machine)
{
	int res;

	if (cpu_initialized) {
		return LOGERR("CPU already initialized");
	}

	awp_enabled = machine->cpu.awp;
	cpu_mod_present = machine->cpu.mod;
	cpu_user_io_illegal = machine->cpu.user_io_illegal;
	nomem_stop = machine->cpu.nomem_stop;
	speed_real = host->emu.speed_real;
	emulation_quantum_ns = 1000 * host->emu.emulation_quantum_us;
	ticks_per_2s = 2000000000L / emulation_quantum_ns;

	sound_enabled = host->sound.enabled;

	res = iset_build(cpu_op_tab, cpu_user_io_illegal);
	if (res != E_OK) {
		return LOGERR("Failed to build CPU instruction table.");
	}

	// this is checked only at power-on
	if (mem_mega_boot()) {
		ic = 0xf000;
	} else {
		ic = 0;
	}
	r[0] = 0;
	SR_WRITE(0);
	int_update_xmask();
	int_clear_all();
	rALARM = false;
	mc = 0;
	cpu_mod_off();
	cpu_state = cp_start_get() ? EM400_STATE_RUN : EM400_STATE_STOP;

	if (clock_init(machine->cpu.clock_period_ms) != E_OK) {
		LOGERR("Failed to initialize clock");
		goto fail;
	}

	if (sound_enabled) {
		if (!speed_real) {
			LOGERR("WARNING: sound won't work with speed_real=false. Buzzer emulation is disabled.");
			sound_enabled = false;
		} else {
			if (buzzer_init(&host->sound) != E_OK) {
				LOGERR("WARNING: could not initialize buzzer; continuing without sound.");
				sound_enabled = false;
			}
		}
	}

	if (pthread_create(&cpu_thread, NULL, cpu_loop, NULL)) {
		LOGERR("Failed to spawn cpu thread.");
		goto fail;
	}
	if (pthread_setname_np(cpu_thread, "cpu")) {
		LOG(L_CPU, "Could not set CPU thread name");
	}

	cpu_initialized = true;
	LOG(L_CPU, "CPU initialized. AWP: %s, modifications: %s, user I/O: %s, stop on nomem: %s",
		awp_enabled ? "enabled" : "disabled",
		cpu_mod_present ? "present" : "absent",
		cpu_user_io_illegal ? "illegal" : "legal",
		nomem_stop ? "true" : "false");
	LOG(L_CPU, "CPU speed: %s, emulation quantum: %i ns",
		speed_real ? "real" : "unlimited",
		emulation_quantum_ns);

	return E_OK;
fail:
	clock_shutdown();
	if (sound_enabled) buzzer_shutdown();
	return E_ERR;
}

// -----------------------------------------------------------------------
void cpu_shutdown()
{
	if (!cpu_initialized) {
		return;
	}

	cpu_state_change(EM400_STATE_OFF, EM400_STATE_ANY);
	pthread_join(cpu_thread, NULL);
	clock_shutdown();
	if (sound_enabled) {
		buzzer_shutdown();
	}
	cpu_initialized = false;
}

// -----------------------------------------------------------------------
void cpu_mod_on()
{
	cpu_mod_active = true;
	clock_set_int(INT_EXTRA);
}

// -----------------------------------------------------------------------
void cpu_mod_off()
{
	cpu_mod_active = false;
	clock_set_int(INT_CLOCK);
}

// -----------------------------------------------------------------------
// clo=false -> MCL instruction executed
// clo=true -> CLEAR key toggled
void cpu_do_clear(bool clo)
{
	// I/O reset should return when we're sure that I/O won't change CPU state (backlogged interrupts, memory writes, ...)
	io_reset();
	mem_reset(clo); // MEGA memory handles manual (long) reset differently
	cpu_mod_off();

	r[0] = 0;
	SR_WRITE(0);
	mc = 0;
	p = false;
	int_update_xmask();
	int_clear_all();

	if (clo) {
		rALARM = false;
	}

	// call even if logging is disabled - user may enable it later
	// and we still want to know if we're running a known OS
	log_check_os();
	log_reset_process();
	log_intlevel_reset();
	log_syscall_reset();
}

// -----------------------------------------------------------------------
static void cpu_do_bin()
{
	int lg = 0;
	uint16_t rb;

	LOG(L_CPU, "Binary load initiated @ 0x%04x", ar);
K1:
	// allow emulation to break free from failed loads with CLEAR
	if (cpu_state_get() == EM400_STATE_CLO) return;

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

	// single writer: load+1+store, not atomic_fetch_add, to avoid a locked RMW every instruction
	atomic_store_explicit(&ips_counter, atomic_load_explicit(&ips_counter, memory_order_relaxed) + 1, memory_order_relaxed);
	instruction_time_ns = 0;

P1:
	cpu_mem_read_1(q, ic, &w);
	ic++;
	if (PR) {
		ir = w;
		PR = false;
	} else {
		ac = w;
		if (!mc) ar = w;
		instruction_time_ns += TIME_MEM_ARG;
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
		instruction_time_ns += TIME_PREMOD;
	} else {
		mc = 0;
		zc17 = false;
	}

// P4 B-mod
	if ((flags & OP_FL_ARG_NORM) && IR_B) {
		zc17 = (ac + r[IR_B]) > 0xffff;
		w = ac + r[IR_B];
		ar = ac = w;
		instruction_time_ns += TIME_BMOD;
	}

// P5 D-mod
	if ((flags & OP_FL_ARG_NORM) && IR_D) {
		cpu_mem_read_1(q, ar, &w);
		ar = ac = w;
		instruction_time_ns += TIME_DMOD;
	}

	if (op->fun != op_77_md) mc = 0;

	LOGDASM((op->flags & (OP_FL_ARG_NORM | OP_FL_ARG_SHORT)), ac, "");
	// execute instruction
	op->fun();
	instruction_time_ns += op->time;

	if (op->fun == op_72_shc) {
		instruction_time_ns += IR_t * TIME_SHIFT;
	} else if (op->fun == op_ou) {
		/*
		 * Minimalistic MULTIX I/O routines (MEGA bootloader specifically)
		 * use an OU immediately followed by HLT to send an I/O command and
		 * immediately start waiting for the interrupt.
		 *
		 * If cpu work scheduler decides to sleep right after the OU
		 * instruction, it may receive the interrupt before
		 * program starts waiting for it with HLT.
		 * Interrupt is then served and HLT waits indefinitely. Program hangs.
		 *
		 * Work around this by recognizing OU+HLT tandems
		 * and skipping scheduler sleep in such condition.
		 * Negative instruction time indicates just that.
		 *
		 * With 100us scheduling granularity it shouldn't be a problem really,
		 * but the cpu work scheduler can sometimes experience latencies
		 * as high as >100ms in virtualized environments.
		 * NOTE: I/O devices have to delay the interrupt anyway,
		 * according to their processing delays.
		*/
		uint16_t tmp;
		mem_read_1(q*nb, ic, &tmp);
		if (cpu_op_tab[tmp]->fun == op_73_hlt) {
			return -instruction_time_ns;
		}
	}

	return instruction_time_ns;

P2: // ineffective and illegal instructions handler
	xi = (flags & OP_FL_ILLEGAL) || (q && (flags & OP_FL_USR_ILLEGAL)) || ((op->fun == op_77_md) && (mc == 3));
	if (xi && !p) {
		// instruction is illegal and skip flag is not set
		LOGDASM(0, 0, "ILL: ");
		int_set(INT_ILLEGAL_INSTRUCTION);
	} else {
		// instruction is ineffective, immediate word argument is skipped
		LOGDASM(0, 0, "NEF: ");
		if ((flags & OP_FL_ARG_NORM) && !IR_C) ic++;
	}
	instruction_time_ns += TIME_P;
	p = false;
	mc = 0;
	return instruction_time_ns;
}

// -----------------------------------------------------------------------
static inline void cpu_latency_stats(int policy, long late_ns)
{
	enum bucket {
		LATE_LT_1US, LATE_LT_10US, LATE_LT_100US,
		LATE_LT_1MS, LATE_LT_10MS, LATE_LT_100MS,
		LATE_GT_100MS,
		BUCKET_COUNT
	};
	static const long thresholds[BUCKET_COUNT] = {
		[LATE_LT_1US]	= 1000L,
		[LATE_LT_10US]	= 10000L,
		[LATE_LT_100US]	= 100000L,
		[LATE_LT_1MS]	= 1000000L,
		[LATE_LT_10MS]	= 10000000L,
		[LATE_LT_100MS]	= 100000000L,
		[LATE_GT_100MS]	= LONG_MAX,
	};

	struct stats {
		unsigned timing_policy[TIMING_POLICY_COUNT];
		unsigned bucket[BUCKET_COUNT];
		unsigned samples;
	} static stats;

	for (enum bucket i=LATE_LT_1US; i<BUCKET_COUNT ; i++) {
		if (late_ns < thresholds[i]) {
			stats.bucket[i]++;
			break;
		}
	}
	stats.samples++;

	stats.timing_policy[policy]++;

	if (stats.samples >= ticks_per_2s) {
		LOG(L_EM4H,
			"<1us: %-6u <10us: %-6u <100us: %-6u"
			" <1ms: %-6u <10ms: %-6u <100ms: %-6u"
			" >=100ms: %-6u"
			" (busy: %u, sleep: %u)",
			stats.bucket[LATE_LT_1US], stats.bucket[LATE_LT_10US], stats.bucket[LATE_LT_100US],
			stats.bucket[LATE_LT_1MS], stats.bucket[LATE_LT_10MS], stats.bucket[LATE_LT_100MS],
			stats.bucket[LATE_GT_100MS],
			stats.timing_policy[0], stats.timing_policy[1] + stats.timing_policy[2]
		);
		stats = (struct stats){0};
	}
}

// -----------------------------------------------------------------------
static inline long cpu_timing_latency(const struct timespec *now, const struct timespec *cpu_timer)
{
	return (now->tv_sec - cpu_timer->tv_sec) * 1000000000L + (now->tv_nsec - cpu_timer->tv_nsec);
}

// -----------------------------------------------------------------------
static inline long cpu_timing_busy_wait(struct timespec *now, const struct timespec *cpu_timer)
{
	long late_ns;
	do {
		CPU_RELAX();
		clock_gettime(CLOCK_MONOTONIC, now);
	} while ((late_ns = cpu_timing_latency(now, cpu_timer)) < 0);
	return late_ns;
}

// -----------------------------------------------------------------------
static inline long cpu_timing_sleep_wait(struct timespec *now, struct timespec *cpu_timer)
{
#ifdef _WIN32
	// winpthreads clock_nanosleep rejects CLOCK_MONOTONIC
	// use a high-res waitable timer instead
	compat_sleep_until(cpu_timer);
#else
	while (clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, cpu_timer, NULL) == EINTR);
#endif
	clock_gettime(CLOCK_MONOTONIC, now);
	return cpu_timing_latency(now, cpu_timer);
}

// -----------------------------------------------------------------------
static void cpu_timekeeping(int cpu_time_ns)
{
	bool skip_sleep = false;

	static enum timing_policy_e timing_policy = TIMING_POLICY_BUSY;
	static int busy_probe_delay = TIMING_PROBE_DELAY;
	static int busy_probes_ok = 0;
	struct timespec now;
	long late_ns;

	if (cpu_time_ns < 0) {
		cpu_time_ns = -cpu_time_ns;
		skip_sleep = true;
	}

	cpu_time_cumulative_ns += cpu_time_ns;

	if (sound_enabled) {
		buzzer_update(ir, cpu_time_ns);
	}

	if (skip_sleep || (cpu_time_cumulative_ns < emulation_quantum_ns)) {
		return;
	}

	cpu_timer.tv_nsec += cpu_time_cumulative_ns;
	while (cpu_timer.tv_nsec >= 1000000000) {
		cpu_timer.tv_nsec -= 1000000000;
		cpu_timer.tv_sec++;
	}
	cpu_time_cumulative_ns = 0;

	// wait
	if (timing_policy == TIMING_POLICY_BUSY) {
		late_ns = cpu_timing_busy_wait(&now, &cpu_timer);
	} else {
		late_ns = cpu_timing_sleep_wait(&now, &cpu_timer);
	}

	cpu_latency_stats(timing_policy, late_ns);

	// decide on the next step
	switch (timing_policy) {
		case TIMING_POLICY_BUSY:
			// count down to the next PROBE
			if (busy_probe_delay > 0) busy_probe_delay--;
			// transition to probe only when both are true:
			//  * no more processing delay (latency < 100us)
			//  * not earlier than 10 BUSY waits
			if ((late_ns < TIMING_BUSY_THRESHOLD_NS) && (busy_probe_delay <= 0)) {
				timing_policy = TIMING_POLICY_PROBE;
			}
			break;
		case TIMING_POLICY_PROBE:
			// probing is successfull if latency is < 500us for 10 consecutive probes
			if (late_ns < TIMING_PROBE_THRESHOLD_NS) {
				busy_probes_ok++;
			} else {
				busy_probes_ok = 0;
			}
			if (busy_probes_ok > TIMING_PROBE_PASSES) {
				// lookin' good, switch to SLEEP
				timing_policy = TIMING_POLICY_SLEEP;
			} else {
				// back to BUSY, count down to the next probe
				timing_policy = TIMING_POLICY_BUSY;
				busy_probe_delay = TIMING_PROBE_DELAY;
			}
			break;
		case TIMING_POLICY_SLEEP:
			// fall back to BUSY immediately if latency spikes (or accumulates) above 1ms
			if (late_ns > TIMING_SLEEP_THRESHOLD_NS) {
				timing_policy = TIMING_POLICY_BUSY;
				busy_probe_delay = TIMING_PROBE_DELAY;
				busy_probes_ok = 0;
			}
			break;
		default:
			break;
	}
}

// -----------------------------------------------------------------------
__attribute__((hot)) static void * cpu_loop(void *ptr)
{
	LOG(L_CPU, "Starting CPU loop");
	bool quit = false;
	clock_gettime(CLOCK_MONOTONIC, &cpu_timer);

	while (!quit) {
		int cpu_time_ns = 0;

		switch (atomic_load_explicit(&cpu_state, memory_order_acquire)) {
			case EM400_STATE_CYCLE:
				cpu_state_change(EM400_STATE_STOP, EM400_STATE_ANY);
				// fallthrough
			case EM400_STATE_RUN:
				if (atomic_load_explicit(&irq, memory_order_acquire) && !p && (mc == 0)) {
					int_serve();
					cpu_time_ns = TIME_INT_SERVE;
				} else {
					cpu_time_ns = cpu_do_cycle();
					if (brk_check()) {
						cpu_state_change(EM400_STATE_STOP, EM400_STATE_ANY);
					}
				}
				break;
			case EM400_STATE_OFF:
				if (sound_enabled) buzzer_stop();
				quit = true;
				break;
			case EM400_STATE_CLO:
				if (sound_enabled) buzzer_stop();
				cpu_do_clear(true);
				cpu_state_change(EM400_STATE_STOP, EM400_STATE_ANY);
				break;
			case EM400_STATE_BIN:
				cpu_do_bin();
				cpu_state_change(EM400_STATE_STOP, EM400_STATE_BIN);
				break;
			case EM400_STATE_LOAD:
				w = kb;
				cpu_do_load(r_selected, w);
				cpu_state_change(EM400_STATE_STOP, EM400_STATE_LOAD);
				break;
			case EM400_STATE_STORE:
				cpu_do_store();
				cpu_state_change(EM400_STATE_STOP, EM400_STATE_STORE);
				break;
			case EM400_STATE_FETCH:
				cpu_do_fetch();
				cpu_state_change(EM400_STATE_STOP, EM400_STATE_FETCH);
				break;
			case EM400_STATE_STOP:
				if (sound_enabled) buzzer_stop();
				int res = cpu_do_wait();
				if (res == EM400_STATE_RUN) {
					if (sound_enabled) buzzer_start();
					clock_gettime(CLOCK_MONOTONIC, &cpu_timer);
					cpu_time_cumulative_ns = 0;
				}
				break;
			case EM400_STATE_WAIT:
				// busy wait to not disturb audio, TODO
				cpu_reg_selected_to_w();
				if (atomic_load_explicit(&irq, memory_order_acquire) && !p && !mc) {
					cpu_state_change(EM400_STATE_RUN, EM400_STATE_WAIT);
				} else {
					cpu_time_ns = emulation_quantum_ns;
				}
				// else = if (!speed_real) {
				//	cpu_do_wait();
				//	cpu_state_change(EM400_STATE_RUN, EM400_STATE_WAIT);
				//}
				break;
		}

		if (speed_real) cpu_timekeeping(cpu_time_ns);
	}

	LOG(L_CPU, "Exiting CPU loop");
	return NULL;
}

// vim: tabstop=4 shiftwidth=4 autoindent
