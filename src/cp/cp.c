//  Copyright (c) 2017-2026 Jakub Filipowicz <jakubf@gmail.com>
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

#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdatomic.h>

#include "libem400.h"
#include "cpu/cpu.h"
#include "cpu/interrupts.h"
#include "mem/mem.h"
#include "io/defs.h"
#include "io/io.h"
#include "utils/utils.h"
#include "log.h"

static atomic_bool start_switch;
static atomic_int reg_select_switch;
static atomic_bool clock_switch;
static atomic_uint kb_switch;

// -----------------------------------------------------------------------
uint16_t cp_bus_w()
{
    return w;
}

// -----------------------------------------------------------------------
void cp_kb_set(uint16_t val)
{
	LOG(L_CPU, "KB := 0x%04x", val);
	atomic_store_explicit(&kb_switch, val, memory_order_relaxed);
	if (cpu_state_get() == EM400_STATE_OFF) return;
	cpu_w_refresh();
}

// -----------------------------------------------------------------------
uint16_t cp_kb_get()
{
	return atomic_load_explicit(&kb_switch, memory_order_relaxed);
}

// -----------------------------------------------------------------------
void cp_start(bool state)
{
	atomic_store_explicit(&start_switch, state, memory_order_relaxed);
	if (cpu_state_get() == EM400_STATE_OFF) return;
	if (state) {
		cpu_state_change(EM400_STATE_RUN, EM400_STATE_STOP);
	} else {
		cpu_state_change(EM400_STATE_STOP, EM400_STATE_ANY);
	}
}

// -----------------------------------------------------------------------
bool cp_start_get()
{
	return atomic_load_explicit(&start_switch, memory_order_relaxed);
}

// -----------------------------------------------------------------------
void cp_cycle()
{
	cpu_state_change(EM400_STATE_CYCLE, EM400_STATE_STOP);
}

// -----------------------------------------------------------------------
void cp_clock_set(int state)
{
	LOG(L_CPU, "Set cpu timer (clock) state: %s", state ? "ON" : "OFF");
	atomic_store_explicit(&clock_switch, state, memory_order_relaxed);
}

// -----------------------------------------------------------------------
int cp_clock_get()
{
	return atomic_load_explicit(&clock_switch, memory_order_relaxed);
}

// -----------------------------------------------------------------------
void cp_clear()
{
	if (cpu_state_get() == EM400_STATE_OFF) return;
	cpu_state_change(EM400_STATE_CLO, EM400_STATE_ANY);
}

// -----------------------------------------------------------------------
void cp_bin()
{
	cpu_state_change(EM400_STATE_BIN, EM400_STATE_STOP);
}

// -----------------------------------------------------------------------
void cp_oprq()
{
	if (cpu_state_get() == EM400_STATE_OFF) return;
	int_set(INT_OPRQ);
}

// -----------------------------------------------------------------------
int cp_stopn(bool state)
{
	// unsupported
	return -1;
}

// -----------------------------------------------------------------------
void cp_reg_select(int reg_id)
{
	LOG(L_CPU, "Selected register on the rotary switch: %s", em400_reg_name(reg_id));
	atomic_store_explicit(&reg_select_switch, reg_id, memory_order_relaxed);
	if (cpu_state_get() == EM400_STATE_OFF) return;
	cpu_w_refresh();
}

// -----------------------------------------------------------------------
int cp_reg_select_get()
{
	return atomic_load_explicit(&reg_select_switch, memory_order_relaxed);
}

// -----------------------------------------------------------------------
bool cp_alarm_get()
{
	return rALARM;
}

// -----------------------------------------------------------------------
bool cp_p_get()
{
	return p;
}

// -----------------------------------------------------------------------
int cp_mc_get()
{
	return mc;
}

// -----------------------------------------------------------------------
bool cp_irq_get()
{
	return irq;
}

// -----------------------------------------------------------------------
bool cp_run_get()
{
	return cpu_state_get() == EM400_STATE_RUN;
}

// -----------------------------------------------------------------------
bool cp_wait_get()
{
	return cpu_state_get() == EM400_STATE_WAIT;
}

// -----------------------------------------------------------------------
bool cp_q_get()
{
	return q;
}

// -----------------------------------------------------------------------
void cp_load()
{
	cpu_state_change(EM400_STATE_LOAD, EM400_STATE_STOP);
}

// -----------------------------------------------------------------------
void cp_fetch()
{
	cpu_state_change(EM400_STATE_FETCH, EM400_STATE_STOP);
}

// -----------------------------------------------------------------------
void cp_store()
{
	cpu_state_change(EM400_STATE_STORE, EM400_STATE_STOP);
}

// vim: tabstop=4 shiftwidth=4 autoindent
