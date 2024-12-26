//  Copyright (c) 2017-2024 Jakub Filipowicz <jakubf@gmail.com>
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

#include "io/defs.h"
#include "io/io.h"
#include "cpu/cp.h"
#include "cpu/cpu.h"
#include "cpu/interrupts.h"
#include "mem/mem.h"
#include "cpu/clock.h"
#include "utils/utils.h"

#include "log.h"

#include "ectl.h" // for global constants/enums

// -----------------------------------------------------------------------
uint16_t cp_bus_w()
{
    return w;
}

// -----------------------------------------------------------------------
void cp_kb_set(uint16_t val)
{
	cpu_kb_set(val);
}

// -----------------------------------------------------------------------
bool cp_mem_read_n(unsigned nb, uint16_t addr, uint16_t *data, unsigned count)
{
	// can't use cpu_mem_read here, as it may fail in "cpu fashion" (with interrupt and all).
	// cp_mem_read() is an emulator extension to the original H/W, used by the interface
	// to display stuff when cpu is in RUN state too, and it could happen that ui tries to
	// read unconfigured memory as well.
	return mem_read_n(nb, addr, data, count);
}

// -----------------------------------------------------------------------
bool cp_mem_write_n(unsigned nb, uint16_t addr, uint16_t *data, unsigned count)
{
	return mem_write_n(nb, addr, data, count);
}

// -----------------------------------------------------------------------
void cp_start(bool state)
{
	if (state) cpu_state_change(ECTL_STATE_RUN, ECTL_STATE_STOP);
	else cpu_state_change(ECTL_STATE_STOP, ECTL_STATE_ANY);
}

// -----------------------------------------------------------------------
void cp_cycle()
{
	cpu_state_change(ECTL_STATE_CYCLE, ECTL_STATE_STOP);
}

// -----------------------------------------------------------------------
void cp_off()
{
	cpu_state_change(ECTL_STATE_OFF, ECTL_STATE_ANY);
}

// -----------------------------------------------------------------------
void cp_clock_set(int state)
{
	clock_set(state);
}

// -----------------------------------------------------------------------
int cp_clock_get()
{
	return clock_get();
}

// -----------------------------------------------------------------------
void cp_clear()
{
	cpu_state_change(ECTL_STATE_CLO, ECTL_STATE_ANY);
}

// -----------------------------------------------------------------------
void cp_bin()
{
	cpu_state_change(ECTL_STATE_BIN, ECTL_STATE_STOP);
}

// -----------------------------------------------------------------------
void cp_oprq()
{
	int_set(INT_OPRQ);
}

// -----------------------------------------------------------------------
unsigned cp_state()
{
	return cpu_state_get();
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
	cpu_reg_select(reg_id);
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
	return cp_state() == ECTL_STATE_RUN;
}

// -----------------------------------------------------------------------
bool cp_wait_get()
{
	return cp_state() == ECTL_STATE_WAIT;
}

// -----------------------------------------------------------------------
bool cp_q_get()
{
	return q;
}

// -----------------------------------------------------------------------
void cp_load()
{
	cpu_state_change(ECTL_STATE_LOAD, ECTL_STATE_STOP);
}

// -----------------------------------------------------------------------
void cp_fetch()
{
	cpu_state_change(ECTL_STATE_FETCH, ECTL_STATE_STOP);
}

// -----------------------------------------------------------------------
void cp_store()
{
	cpu_state_change(ECTL_STATE_STORE, ECTL_STATE_STOP);
}

// vim: tabstop=4 shiftwidth=4 autoindent
