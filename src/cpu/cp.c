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

#include "cpu/cp.h"
#include "cpu/cpu.h"
#include "cpu/interrupts.h"
#include "mem/mem.h"
#include "cpu/timer.h"
#include "fpga/iobus.h"

#include "cfg.h"
#include "log.h"

static int fpga;

// -----------------------------------------------------------------------
int cp_init(struct cfg_em400 *cfg)
{
	fpga = cfg->fpga;
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
		if (id <= CP_REG_KB2) {
			iob_cp_set_rotary(id);
			stat = iob_cp_get_status();
			reg = stat->data;
			free(stat);
		} else {
			stat = iob_cp_get_status();
			reg = stat->leds;
			free(stat);
			if ((id == CP_REG_MOD) || (id == CP_REG_MODc)) {
				reg = (reg & IOB_LED_MODE) >> 9;
			} else if (id == CP_REG_ALARM) {
				reg = reg & IOB_LED_ALARM;
			} else {
				return -1;
			}
		}
	} else {
		switch (id) {
			case CP_REG_R0:
			case CP_REG_R1:
			case CP_REG_R2:
			case CP_REG_R3:
			case CP_REG_R4:
			case CP_REG_R5:
			case CP_REG_R6:
			case CP_REG_R7: reg = regs[id]; break;
			case CP_REG_IC: reg = rIC; break;
			// n/a case CP_REG_AC:
			// n/a case CP_REG_AR:
			case CP_REG_IR: reg = rIR; break;
			case CP_REG_SR: reg = rSR; break;
			// n/a case CP_REG_RZ:
			case CP_REG_KB: reg = rKB; break;
			case CP_REG_KB2: reg = rKB; break;
			case CP_REG_MOD: reg = rMOD; break;
			case CP_REG_MODc: reg = rMODc; break;
			case CP_REG_ALARM: reg = rALARM; break;
			default: return -1;
		}
	}
	return reg;
}

// -----------------------------------------------------------------------
int cp_reg_set(unsigned id, uint16_t v)
{
	if (fpga) {
		if (id >= CP_REG_KB2) {
			return -1;
		}
		iob_cp_set_rotary(id);
		iob_cp_set_keys(v);
		iob_cp_set_fn(IOB_FN_LOAD, 1);
	} else {
		switch (id) {
			case CP_REG_R0:
			case CP_REG_R1:
			case CP_REG_R2:
			case CP_REG_R3:
			case CP_REG_R4:
			case CP_REG_R5:
			case CP_REG_R6:
			case CP_REG_R7: regs[id] = v; break;
			case CP_REG_IC: rIC = v; break;
			case CP_REG_AC: return -1;
			case CP_REG_AR: return -1;
			case CP_REG_IR: rIR = v; break;
			case CP_REG_SR: rSR = v; break;
			case CP_REG_RZ: return -1;
			case CP_REG_KB: rKB = v; break;
			case CP_REG_KB2: rKB = v; break;
			case CP_REG_MOD: rMOD = v; break;
			case CP_REG_MODc: rMODc = v; break;
			case CP_REG_ALARM: rALARM = v; break;
			default: return -1;
		}
	}
	return 0;
}

// -----------------------------------------------------------------------
int cp_mem_mget(unsigned nb, uint16_t addr, uint16_t *data, unsigned count)
{
	uint16_t old_sr = 0;
	struct iob_cp_status *stat;

	if (fpga) {
		if (nb) {
			iob_cp_set_rotary(CP_REG_SR);
			stat = iob_cp_get_status();
			old_sr = stat->data;
			free(stat);
			uint16_t sr = 0b0000000000100000 | (nb & 0b1111);
			iob_cp_set_keys(sr);
			iob_cp_set_fn(IOB_FN_LOAD, 1);
		}
		iob_cp_set_rotary(CP_REG_AR);
		iob_cp_set_keys(addr);
		iob_cp_set_fn(IOB_FN_LOAD, 1);
		iob_cp_set_rotary(CP_REG_AC);
		for (unsigned i=0 ; i<count ; i++) {
			iob_cp_set_fn(IOB_FN_FETCH, 1);
			stat = iob_cp_get_status();
			*(data+i) = stat->data;
			free(stat);
		}
		if (nb) {
			iob_cp_set_rotary(CP_REG_SR);
			iob_cp_set_keys(old_sr);
			iob_cp_set_fn(IOB_FN_LOAD, 1);
		}
		return count;
	} else {
		return mem_mget(nb, addr, data, count);
	}
}

// -----------------------------------------------------------------------
int cp_mem_mput(unsigned nb, uint16_t addr, uint16_t *data, unsigned count)
{
	uint16_t old_sr = 0;
	struct iob_cp_status *stat;

	if (fpga) {
		if (nb) {
			iob_cp_set_rotary(CP_REG_SR);
			stat = iob_cp_get_status();
			old_sr = stat->data;
			free(stat);
			uint16_t sr = 0b0000000000100000 | (nb & 0b1111);
			iob_cp_set_keys(sr);
			iob_cp_set_fn(IOB_FN_LOAD, 1);
		}

		iob_cp_set_rotary(CP_REG_AR);
		iob_cp_set_keys(addr);
		iob_cp_set_fn(IOB_FN_LOAD, 1);
		iob_cp_set_rotary(CP_REG_KB);
		for (unsigned i=0 ; i<count ; i++) {
			iob_cp_set_keys(*(data+i));
			iob_cp_set_fn(IOB_FN_STORE, 1);
		}
		if (nb) {
			iob_cp_set_rotary(CP_REG_SR);
			iob_cp_set_keys(old_sr);
			iob_cp_set_fn(IOB_FN_LOAD, 1);
		}
		return count;
	} else {
		return mem_mput(nb, addr, data, count);
	}
}

// -----------------------------------------------------------------------
void cp_stop()
{
	if (fpga) {
		iob_cp_set_fn(IOB_FN_START, 0);
	} else {
		cpu_trigger_state(STATE_STOP);
	}
}

// -----------------------------------------------------------------------
void cp_start()
{
	if (fpga) {
		iob_cp_set_fn(IOB_FN_START, 1);
	} else {
		cpu_clear_state(STATE_STOP);
	}
}

// -----------------------------------------------------------------------
void cp_cycle()
{
	if (fpga) {
		iob_cp_set_fn(IOB_FN_CYCLE, 1);
	} else {
		cpu_trigger_cycle();
	}
}

// -----------------------------------------------------------------------
void cp_off()
{
	if (fpga) {
		iob_quit();
	} else {
		cpu_trigger_state(STATE_QUIT);
	}
}

// -----------------------------------------------------------------------
void cp_clock_set(int state)
{
	if (fpga) {
		iob_cp_set_fn(IOB_FN_CLOCK, state);
	} else {
		if (state == 0) {
			timer_off();
		} else {
			timer_on();
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
		state = timer_get_state();
	}

	return state;
}

// -----------------------------------------------------------------------
void cp_clear()
{
	if (fpga) {
		iob_cp_set_fn(IOB_FN_CLEAR, 1);
	} else {
		cpu_trigger_state(STATE_CLO);
	}
}

// -----------------------------------------------------------------------
void cp_bin()
{

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
int cp_int_set(unsigned i)
{
	if (i < 32) {
		int_set(i);
		return 0;
	} else {
		return -1;
	}
}

// -----------------------------------------------------------------------
uint32_t cp_int_get()
{
	uint32_t rz;
	if (fpga) {
		iob_cp_set_rotary(CP_REG_RZ);
        struct iob_cp_status *stat = iob_cp_get_status();
        rz = (((uint32_t)stat->data & 0b1111111111110000) << 16) | (stat->data & 0b1111);
        free(stat);
	} else {
		rz = RZ;
	}
	return rz;
}

// -----------------------------------------------------------------------
int cp_state()
{
	int status = 0;

	if (fpga) {
		struct iob_cp_status *stat = iob_cp_get_status();
		if (stat->leds & IOB_LED_WAIT) status |= STATE_HALT;
		if (!(stat->leds & IOB_LED_RUN)) status |= STATE_STOP;
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
		iob_cp_set_rotary(CP_REG_KB);
		iob_cp_set_keys(addr);
		iob_cp_set_fn(IOB_FN_STOPN, 1);
		return 0;
	} else {
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
		return -1;
	}
}

// vim: tabstop=4 shiftwidth=4 autoindent
