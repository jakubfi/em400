//  Copyright (c) 2017 Jakub Filipowicz <jakubf@gmail.com>
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
#include "fpga/iobus.h"
#include "utils/utils.h"

#include "cfg.h"
#include "log.h"

#include "ectl.h" // for global constants/enums

static bool fpga;

// -----------------------------------------------------------------------
int cp_init(em400_cfg *cfg)
{
	fpga = cfg_getbool(cfg, "cpu:fpga", CFG_DEFAULT_CPU_FPGA);
	return E_OK;
}

// -----------------------------------------------------------------------
void cp_shutdown()
{

}

// -----------------------------------------------------------------------
int cp_bus_w_get()
{
    return w;
}

// -----------------------------------------------------------------------
void cp_kb_set(uint16_t val)
{
	if (fpga) {
		iob_cp_set_keys(val);
	} else {
		// NOTE: force=true because this is possible even when CPU is running
		cpu_register_load(ECTL_REG_KB, val, true);
	}
}

// -----------------------------------------------------------------------
int cp_reg_get(unsigned id)
{
	int reg = -1;
	struct iob_cp_status *stat;

	if (fpga) {
		iob_cp_set_rotary(id);
		stat = iob_cp_get_status();
		reg = stat->data;
		free(stat);
	} else {
		switch (id) {
			case ECTL_REG_R0:
			case ECTL_REG_R1:
			case ECTL_REG_R2:
			case ECTL_REG_R3:
			case ECTL_REG_R4:
			case ECTL_REG_R5:
			case ECTL_REG_R6:
			case ECTL_REG_R7: reg = r[id]; break;
			case ECTL_REG_IC: reg = ic; break;
			case ECTL_REG_AC: reg = ac; break;
			case ECTL_REG_AR: reg = ar; break;
			case ECTL_REG_IR: reg = ir; break;
			case ECTL_REG_SR: reg = SR_READ(); break;
			case ECTL_REG_RZ: reg = int_get_nchan(); break;
			case ECTL_REG_KB:
			case ECTL_REG_KB2: reg = kb; break;
			default: reg = -1; break;
		}
	}
	return reg;
}

// -----------------------------------------------------------------------
int cp_reg_set(unsigned id, uint16_t v)
{
	if (fpga) {
		iob_cp_set_rotary(id);
		iob_cp_set_keys(v);
		iob_cp_set_fn(IOB_FN_LOAD, 1);
	} else {
		cpu_register_load(id, v, false);
	}
	return 0;
}

// -----------------------------------------------------------------------
bool cp_mem_read_n(unsigned nb, uint16_t addr, uint16_t *data, unsigned count)
{
	uint16_t old_sr = 0;
	struct iob_cp_status *stat;

	if (fpga) {
		if (nb) {
			iob_cp_set_rotary(ECTL_REG_SR);
			stat = iob_cp_get_status();
			old_sr = stat->data;
			free(stat);
			uint16_t sr = 0b0000000000100000 | (nb & 0b1111);
			iob_cp_set_keys(sr);
			iob_cp_set_fn(IOB_FN_LOAD, 1);
		}
		iob_cp_set_rotary(ECTL_REG_AR);
		iob_cp_set_keys(addr);
		iob_cp_set_fn(IOB_FN_LOAD, 1);
		iob_cp_set_rotary(ECTL_REG_AC);
		for (unsigned i=0 ; i<count ; i++) {
			iob_cp_set_fn(IOB_FN_FETCH, 1);
			stat = iob_cp_get_status();
			*(data+i) = stat->data;
			free(stat);
		}
		if (nb) {
			iob_cp_set_rotary(ECTL_REG_SR);
			iob_cp_set_keys(old_sr);
			iob_cp_set_fn(IOB_FN_LOAD, 1);
		}
		return true;
	} else {
		// TODO: can't use cpu_mem_read, as it may fail in "cpu fashion" (with interrupt and all).
		// cp_mem_read() may be used by the interface to display stuff when cpu is RUN state too,
		// and it could happen that ui tries to read unconfigured memory as well.
		// On the other hand, access from control panel is a regular CPU memory access,
		// thus it should use cpu_mem_get() and fail on unconfigured memory...
		return mem_read_n(nb, addr, data, count);
	}
}

// -----------------------------------------------------------------------
bool cp_mem_write_n(unsigned nb, uint16_t addr, uint16_t *data, unsigned count)
{
	uint16_t old_sr = 0;
	struct iob_cp_status *stat;

	if (fpga) {
		if (nb) {
			iob_cp_set_rotary(ECTL_REG_SR);
			stat = iob_cp_get_status();
			old_sr = stat->data;
			free(stat);
			uint16_t sr = 0b0000000000100000 | (nb & 0b1111);
			iob_cp_set_keys(sr);
			iob_cp_set_fn(IOB_FN_LOAD, 1);
		}

		iob_cp_set_rotary(ECTL_REG_AR);
		iob_cp_set_keys(addr);
		iob_cp_set_fn(IOB_FN_LOAD, 1);
		iob_cp_set_rotary(ECTL_REG_KB);
		for (unsigned i=0 ; i<count ; i++) {
			iob_cp_set_keys(*(data+i));
			iob_cp_set_fn(IOB_FN_STORE, 1);
		}
		if (nb) {
			iob_cp_set_rotary(ECTL_REG_SR);
			iob_cp_set_keys(old_sr);
			iob_cp_set_fn(IOB_FN_LOAD, 1);
		}
		return true;
	} else {
		return mem_write_n(nb, addr, data, count);
	}
}

// -----------------------------------------------------------------------
void cp_start(bool state)
{
	if (fpga) {
		iob_cp_set_fn(IOB_FN_START, state);
	} else {
		if (state) cpu_state_change(ECTL_STATE_RUN, ECTL_STATE_STOP);
		else cpu_state_change(ECTL_STATE_STOP, -1);
	}
}

// -----------------------------------------------------------------------
void cp_cycle()
{
	if (fpga) {
		iob_cp_set_fn(IOB_FN_CYCLE, 1);
	} else {
		cpu_state_change(ECTL_STATE_CYCLE, ECTL_STATE_STOP);
	}
}

// -----------------------------------------------------------------------
void cp_off()
{
	if (fpga) {
		iob_quit();
	} else {
		cpu_state_change(ECTL_STATE_OFF, -1);
	}
}

// -----------------------------------------------------------------------
void cp_clock_set(int state)
{
	if (fpga) {
		iob_cp_set_fn(IOB_FN_CLOCK, state);
	} else {
		if (state == 0) {
			clock_off();
		} else {
			clock_on();
		}
	}
}

// -----------------------------------------------------------------------
int cp_clock_get()
{
	int state;
	if (fpga) {
		struct iob_cp_status *stat = iob_cp_get_status();
		state = stat->leds & IOB_LED_CLOCK ? 1 : 0;
		free(stat);
	} else {
		state = clock_get_state();
	}

	return state;
}

// -----------------------------------------------------------------------
void cp_clear()
{
	if (fpga) {
		iob_cp_set_fn(IOB_FN_CLEAR, 1);
	} else {
		cpu_state_change(ECTL_STATE_CLO, -1);
	}
}

// -----------------------------------------------------------------------
int cp_bin()
{
	int res = 1;

	if (fpga) {
		// unsupported
	} else {
		res = cpu_state_change(ECTL_STATE_BIN, ECTL_STATE_STOP);
	}

	return res;
}

// -----------------------------------------------------------------------
void cp_oprq()
{
	if (fpga) {
		iob_cp_set_fn(IOB_FN_OPRQ, 1);
	} else {
		int_set(INT_OPRQ);
	}
}

// -----------------------------------------------------------------------
int cp_state()
{
	int status = 0;

	if (fpga) {
		struct iob_cp_status *stat = iob_cp_get_status();
		if (stat->leds & IOB_LED_WAIT) status |= ECTL_STATE_WAIT;
		if (!(stat->leds & IOB_LED_RUN)) status |= ECTL_STATE_STOP;
		free(stat);
	} else {
		status = cpu_state_get();
	}

	return status;
}

// -----------------------------------------------------------------------
int cp_stopn(bool state)
{
	if (fpga) {
		iob_cp_set_fn(IOB_FN_STOPN, state);
		return 0;
	} else {
		// unsupported
		return -1;
	}
}

// -----------------------------------------------------------------------
void cp_reg_select(int id)
{
	if (fpga) {
		iob_cp_set_rotary(id);
	} else {
		cpu_reg_select(id);
	}
}

// -----------------------------------------------------------------------
bool cp_alarm_get()
{
	if (fpga) {
		struct iob_cp_status *stat = iob_cp_get_status();
		bool val = stat->leds & IOB_LED_ALARM;
		free(stat);
		return val;
	} else {
		return rALARM;
	}
}

// -----------------------------------------------------------------------
bool cp_p_get()
{
	if (fpga) {
		struct iob_cp_status *stat = iob_cp_get_status();
		bool val = stat->leds & IOB_LED_P;
		free(stat);
		return val;
	} else {
		return p;
	}
}

// -----------------------------------------------------------------------
int cp_mc_get()
{
	if (fpga) {
		struct iob_cp_status *stat = iob_cp_get_status();
		bool val = stat->leds & IOB_LED_MC;
		free(stat);
		return val;
	} else {
		return mc;
	}
}

// -----------------------------------------------------------------------
bool cp_irq_get()
{
	if (fpga) {
		struct iob_cp_status *stat = iob_cp_get_status();
		bool val = stat->leds & IOB_LED_IRQ;
		free(stat);
		return val;
	} else {
		return rp != 0;
	}
}

// -----------------------------------------------------------------------
bool cp_run_get()
{
	if (fpga) {
		struct iob_cp_status *stat = iob_cp_get_status();
		bool val = stat->leds & IOB_LED_RUN;
		free(stat);
		return val;
	} else {
		return cp_state() == ECTL_STATE_RUN;
	}
}

// -----------------------------------------------------------------------
bool cp_wait_get()
{
	if (fpga) {
		struct iob_cp_status *stat = iob_cp_get_status();
		bool val = stat->leds & IOB_LED_WAIT;
		free(stat);
		return val;
	} else {
		return cp_state() == ECTL_STATE_WAIT;
	}
}

// -----------------------------------------------------------------------
uint16_t cp_int_get_chan()
{
	return int_get_chan();
}

// -----------------------------------------------------------------------
bool cp_q_get()
{
	return cp_reg_get(ECTL_REG_SR) & 0b100000;
}

// -----------------------------------------------------------------------
int cp_nb_get()
{
	return cp_reg_get(ECTL_REG_SR) & 0b1111;
}

// -----------------------------------------------------------------------
int cp_qnb_get()
{
	return cp_q_get() * cp_nb_get();
}
// TODO: add new _LOAD, ... states and
// move load/store/fetch execution to cpu loop, like bin?
// -----------------------------------------------------------------------
void cp_load()
{
	cpu_register_load(r_selected, kb, false);
}

// -----------------------------------------------------------------------
void cp_fetch()
{
	cpu_fetch();
}

// -----------------------------------------------------------------------
void cp_store()
{
	cpu_store();
}

// vim: tabstop=4 shiftwidth=4 autoindent
