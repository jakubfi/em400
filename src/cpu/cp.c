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
int cp_reg_get(unsigned id)
{
	int reg = -1;
	struct iob_cp_status *stat;

	if (fpga) {
		if (id <= ECTL_REG_KB2) {
			iob_cp_set_rotary(id);
			stat = iob_cp_get_status();
			reg = stat->data;
			free(stat);
		} else {
			stat = iob_cp_get_status();
			reg = stat->leds;
			free(stat);
			if (id == ECTL_REG_MC) {
				reg = (reg & IOB_LED_MODE) >> 9;
			} else if (id == ECTL_REG_ALARM) {
				reg = reg & IOB_LED_ALARM;
			} else if (id == ECTL_REG_P) {
				reg = reg & IOB_LED_P;
			} else {
				return -1;
			}
		}
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
			case ECTL_REG_MC: reg = mc; break;
			case ECTL_REG_ALARM: reg = rALARM; break;
			case ECTL_REG_RM: reg = rm; break;
			case ECTL_REG_Q: reg = q; break;
			case ECTL_REG_BS: reg = bs; break;
			case ECTL_REG_NB: reg = nb; break;
			case ECTL_REG_P: reg = p; break;
			case ECTL_REG_RZ_IO: reg = int_get_chan(); break;
			default: reg = -1; break;
		}
	}
	return reg;
}

// -----------------------------------------------------------------------
int cp_reg_set(unsigned id, uint16_t v)
{
	if (fpga) {
		if (id >= ECTL_REG_KB2) {
			return -1;
		}
		iob_cp_set_rotary(id);
		iob_cp_set_keys(v);
		iob_cp_set_fn(IOB_FN_LOAD, 1);
	} else {
		switch (id) {
			case ECTL_REG_R0:
			case ECTL_REG_R1:
			case ECTL_REG_R2:
			case ECTL_REG_R3:
			case ECTL_REG_R4:
			case ECTL_REG_R5:
			case ECTL_REG_R6:
			case ECTL_REG_R7: r[id] = v; break;
			case ECTL_REG_IC: ic = v; break;
			case ECTL_REG_AC: ac = v; break;
			case ECTL_REG_AR: ar = v ; break;
			case ECTL_REG_IR: ir = v; break;
			case ECTL_REG_SR: SR_WRITE(v); break;
			// n/a case ECTL_REG_RZ:
			case ECTL_REG_KB:
			case ECTL_REG_KB2: kb = v; break;
			case ECTL_REG_MC: mc = v; break;
			case ECTL_REG_ALARM: rALARM = v; break;
			case ECTL_REG_RM: rm = v & 0b1111111111; break;
			case ECTL_REG_Q: q = v; break;
			case ECTL_REG_BS: bs = v; break;
			case ECTL_REG_NB: nb = v & 0b1111; break;
			case ECTL_REG_P: p = v; break;
			// n/a case ECTL_REG_RZ_IO:
			default: return -1;
		}
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
void cp_stop()
{
	if (fpga) {
		iob_cp_set_fn(IOB_FN_START, 0);
	} else {
		cpu_state_change(ECTL_STATE_STOP, -1);
	}
}

// -----------------------------------------------------------------------
void cp_start()
{
	if (fpga) {
		iob_cp_set_fn(IOB_FN_START, 1);
	} else {
		cpu_state_change(ECTL_STATE_RUN, ECTL_STATE_STOP);
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
int cp_stopn(uint16_t addr)
{
	if (fpga) {
		iob_cp_set_rotary(ECTL_REG_KB);
		iob_cp_set_keys(addr);
		iob_cp_set_fn(IOB_FN_STOPN, 1);
		return 0;
	} else {
		// unsupported
		return -1;
	}
}

// -----------------------------------------------------------------------
int cp_stopn_off()
{
	if (fpga) {
		iob_cp_set_fn(IOB_FN_STOPN, 0);
		return 0;
	} else {
		// unsupported
		return -1;
	}
}

// vim: tabstop=4 shiftwidth=4 autoindent
