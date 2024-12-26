//  Copyright (c) 2024 Jakub Filipowicz <jakubf@gmail.com>
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

#include <strings.h>

#include "mem/mem.h"
#include "cpu/cpu.h"
#include "cpu/interrupts.h"
#include "cpu/cp.h"

const char *em400_reg_names[] = {
	"R0", "R1", "R2", "R3", "R4", "R5", "R6", "R7",
	"IC", "AC", "AR", "IR", "SR", "RZ", "KB", "KB",
	"??"
};

// -----------------------------------------------------------------------
// --- MAINTENANCE -------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
int em400_mem_configure(struct em400_cfg_mem *c_mem)
{
	return mem_configure(c_mem);
}

// -----------------------------------------------------------------------
int em400_cpu_configure(struct em400_cfg_cpu *c_cpu, struct em400_cfg_buzzer *c_buzzer)
{
	return cpu_configure(c_cpu, c_buzzer);
}

// -----------------------------------------------------------------------
const char * em400_version()
{
	static const char *ver = EM400_VERSION;
	return ver;
}

// -----------------------------------------------------------------------
// --- CONTROL PANEL -----------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
uint16_t em400_cp_w_leds()
{
	return cp_bus_w();
}

// -----------------------------------------------------------------------
bool em400_cp_run_led()
{
	return cp_run_get();
}

// -----------------------------------------------------------------------
bool em400_cp_wait_led()
{
	return cp_wait_get();
}

// -----------------------------------------------------------------------
bool em400_cp_alarm_led()
{
	return cp_alarm_get();
}

// -----------------------------------------------------------------------
bool em400_cp_clock_led()
{
	return cp_clock_get();
}

// -----------------------------------------------------------------------
bool em400_cp_q_led()
{
	return cp_q_get();
}

// -----------------------------------------------------------------------
bool em400_cp_p_led()
{
	return cp_p_get();
}

// -----------------------------------------------------------------------
bool em400_cp_mc_led()
{
	return cp_mc_get();
}

// -----------------------------------------------------------------------
bool em400_cp_irq_led()
{
	return cp_irq_get();
}

// -----------------------------------------------------------------------
void em400_cp_kb(uint16_t val)
{
	cp_kb_set(val);
}

// -----------------------------------------------------------------------
void em400_cp_stopn(bool state)
{
	cp_stopn(state);
}

// -----------------------------------------------------------------------
void em400_cp_cycle()
{
	cp_cycle();
}

// -----------------------------------------------------------------------
void em400_cp_load()
{
	cp_load();
}

// -----------------------------------------------------------------------
void em400_cp_store()
{
	cp_store();
}

// -----------------------------------------------------------------------
void em400_cp_fetch()
{
	cp_fetch();
}

// -----------------------------------------------------------------------
void em400_cp_start(bool state)
{
	cp_start(state);
}

// -----------------------------------------------------------------------
void em400_cp_bin()
{
	cp_bin();
}

// -----------------------------------------------------------------------
void em400_cp_clear()
{
	cp_clear();
}

// -----------------------------------------------------------------------
void em400_cp_clock(int state)
{
	cp_clock_set(state);
}

// -----------------------------------------------------------------------
void em400_cp_oprq()
{
	cp_oprq();
}

// -----------------------------------------------------------------------
void em400_cp_reg_select(int reg_id)
{
	cp_reg_select(reg_id);
}

// -----------------------------------------------------------------------
// --- EM400 EXTENSIONS --------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
int em400_reg(unsigned reg_id)
{
	return cpu_reg_fetch(reg_id);
}

// -----------------------------------------------------------------------
void em400_regs(uint16_t *dest)
{
	for (int i=0 ; i<EM400_REG_COUNT ; i++) {
		dest[i] = cpu_reg_fetch(i);
	}
}

// -----------------------------------------------------------------------
void em400_reg_set(unsigned reg_id, uint16_t val)
{
	cpu_reg_load(reg_id, val);
}

// -----------------------------------------------------------------------
unsigned int em400_reg_id(char *name)
{
	const char **rname = em400_reg_names;
	int idx = 0;
	while (idx < EM400_REG_COUNT) {
		if (!strcasecmp(name, *rname)) {
			return idx;
		}
		idx++;
		rname++;
	}

	return -1;
}

// -----------------------------------------------------------------------
const char * em400_reg_name(unsigned reg_id)
{
	if (reg_id < EM400_REG_COUNT) {
		return em400_reg_names[reg_id];
	} else {
		return em400_reg_names[EM400_REG_COUNT];
	}
}

// -----------------------------------------------------------------------
unsigned em400_nb()
{
	return nb;
}

// -----------------------------------------------------------------------
unsigned em400_qnb()
{
	return q * nb;
}

// -----------------------------------------------------------------------
uint32_t em400_rz32()
{
	return rz;
}

// -----------------------------------------------------------------------
int em400_int_set(unsigned interrupt)
{
	if (interrupt >= 32) {
		return -1;
	}
	int_set(interrupt);
	return 0;
}

// -----------------------------------------------------------------------
int em400_int_clear(unsigned interrupt)
{
	if (interrupt >= 32) {
		return -1;
	}
	int_clear(interrupt);
	return 0;
}

// vim: tabstop=4 shiftwidth=4 autoindent
