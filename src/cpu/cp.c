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

static int fpga;

// -----------------------------------------------------------------------
int cp_init(dictionary *cfg)
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
			if ((id == ECTL_REG_MOD) || (id == ECTL_REG_MODc)) {
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
			case ECTL_REG_R7: reg = regs[id]; break;
			case ECTL_REG_IC: reg = rIC; break;
			// n/a case ECTL_REG_AC:
			// n/a case ECTL_REG_AR:
			case ECTL_REG_IR: reg = rIR; break;
			case ECTL_REG_SR: reg = SR_read(); break;
			case ECTL_REG_RZ: reg = int_get_nchan(); break;
			case ECTL_REG_KB: reg = rKB; break;
			case ECTL_REG_KB2: reg = rKB; break;
			case ECTL_REG_MOD: reg = rMOD; break;
			case ECTL_REG_MODc: reg = rMODc; break;
			case ECTL_REG_ALARM: reg = rALARM; break;
			case ECTL_REG_RM: reg = RM; break;
			case ECTL_REG_Q: reg = Q; break;
			case ECTL_REG_BS: reg = BS; break;
			case ECTL_REG_NB: reg = NB; break;
			case ECTL_REG_P: reg = P; break;
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
			case ECTL_REG_R7: regs[id] = v; break;
			case ECTL_REG_IC: rIC = v; break;
			// n/a case ECTL_REG_AC:
			// n/a case ECTL_REG_AR:
			case ECTL_REG_IR: rIR = v; break;
			case ECTL_REG_SR: SR_write(v); break;
			// n/a case ECTL_REG_RZ:
			case ECTL_REG_KB: rKB = v; break;
			case ECTL_REG_KB2: rKB = v; break;
			case ECTL_REG_MOD: rMOD = v; break;
			case ECTL_REG_MODc: rMODc = v; break;
			case ECTL_REG_ALARM: rALARM = v; break;
			case ECTL_REG_RM: RM = v & 0b1111111111; break;
			case ECTL_REG_Q: Q = v ? 1 : 0; break;
			case ECTL_REG_BS: BS = v ? 1 : 0; break;
			case ECTL_REG_NB: NB = v & 0b1111; break;
			case ECTL_REG_P: P = v ? 1 : 0; break;
			// n/a case ECTL_REG_RZ_IO:
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
		cpu_trigger_state(ECTL_STATE_STOP);
	}
}

// -----------------------------------------------------------------------
void cp_start()
{
	if (fpga) {
		iob_cp_set_fn(IOB_FN_START, 1);
	} else {
		cpu_clear_state(ECTL_STATE_STOP | ECTL_STATE_BRK);
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
		cpu_trigger_state(ECTL_STATE_OFF);
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
		cpu_trigger_state(ECTL_STATE_CLO);
	}
}

// -----------------------------------------------------------------------
int cp_bin(uint16_t ar)
{
	uint16_t addr = ar;
	int words = 0;
	uint16_t data;
	uint8_t bdata[3];
	int cnt = 0;
	int res;

	if (fpga) {
		// unsupported
	} else {
		while (1) {
			res = io_dispatch(IO_IN, rKB, &data);
			if (res != IO_OK) {
				continue;
			}
			bdata[cnt] = data & 0xff;
			if ((cnt == 0) && bin_is_end(bdata[cnt])) {
				break;
			} else if (bin_is_valid(bdata[cnt])) {
				cnt++;
				if (cnt >= 3) {
					cnt = 0;
					if (cpu_mem_put(0, addr, bin2word(bdata)) == 0) {
						break;
					}
					addr++;
					words++;
				}
			}
		}
	}

	return words;
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
