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

#include "cpu/cp.h"
#include "cpu/cpu.h"
#include "cpu/interrupts.h"
#include "mem/mem.h"
#include "cpu/timer.h"
#include "cfg.h"
#include "log.h"

// -----------------------------------------------------------------------
int cp_init(struct cfg_em400 *cfg)
{
	return E_OK;
}

// -----------------------------------------------------------------------
void cp_shutdown()
{

}

// -----------------------------------------------------------------------
int cp_reg_get(unsigned id)
{
	switch (id) {
		case CP_REG_R0:
		case CP_REG_R1:
		case CP_REG_R2:
		case CP_REG_R3:
		case CP_REG_R4:
		case CP_REG_R5:
		case CP_REG_R6:
		case CP_REG_R7: return regs[id];
		case CP_REG_IC: return rIC;
		case CP_REG_AC: return -1;
		case CP_REG_AR: return -1;
		case CP_REG_IR: return rIR;
		case CP_REG_SR: return rSR;
		case CP_REG_RZ: return -1;
		case CP_REG_KB: return rKB;
		case CP_REG_KB2: return rKB;
		case CP_REG_MOD: return rMOD;
		case CP_REG_MODc: return rMODc;
		case CP_REG_ALARM: return rALARM;
		default: return -1;
	}
}

// -----------------------------------------------------------------------
int cp_reg_set(unsigned id, uint16_t v)
{
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
	return 0;
}

// -----------------------------------------------------------------------
int cp_mem_get(int nb, uint16_t addr, uint16_t *data)
{
	return mem_get(nb, addr, data);
}

// -----------------------------------------------------------------------
int cp_mem_put(int nb, uint16_t addr, uint16_t data)
{
	return mem_put(nb, addr, data);
}

// -----------------------------------------------------------------------
void cp_stop()
{
	cpu_trigger_state(STATE_STOP);
}

// -----------------------------------------------------------------------
void cp_start()
{
	cpu_clear_state(STATE_STOP);
}

// -----------------------------------------------------------------------
void cp_cycle()
{
	cpu_trigger_cycle();
}

// -----------------------------------------------------------------------
void cp_off()
{
	cpu_trigger_state(STATE_QUIT);
}

// -----------------------------------------------------------------------
void cp_clock_set(int state)
{
	if (state == 0) {
		timer_off();
	} else {
		timer_on();
	}
}

// -----------------------------------------------------------------------
int cp_clock_get()
{
	return timer_get_state();
}

// -----------------------------------------------------------------------
void cp_clear()
{
	cpu_trigger_state(STATE_CLO);
}

// -----------------------------------------------------------------------
void cp_bin()
{

}

// -----------------------------------------------------------------------
void cp_oprq()
{
	int_set(INT_OPRQ);
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
	return RZ;
}

// -----------------------------------------------------------------------
int cp_state()
{
	return cpu_state_get();
}

// vim: tabstop=4 shiftwidth=4 autoindent
