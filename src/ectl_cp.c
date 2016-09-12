//  Copyright (c) 2016 Jakub Filipowicz <jakubf@gmail.com>
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
#include <inttypes.h>
#include <string.h>

#include "atomic.h"
#include "ectl_emu.h"
#include "cpu/cpu.h"
#include "cpu/timer.h"
#include "cpu/interrupts.h"
#include "mem/mem.h"

// -----------------------------------------------------------------------
void ectl_regs_get(uint16_t *dregs)
{
	atom_fence();
	memcpy(dregs, regs, 8 * sizeof(uint16_t));
}

// -----------------------------------------------------------------------
uint16_t ectl_reg_get(int reg)
{
	atom_fence();
	return regs[reg];
}

// -----------------------------------------------------------------------
void ectl_reg_set(int reg, uint16_t val)
{
	regs[reg] = val;
	atom_fence();
}

// -----------------------------------------------------------------------
int ectl_mem_get(int nb, uint16_t addr, uint16_t *dest, int count)
{
	return mem_mget(nb, addr, dest, count);
}

// -----------------------------------------------------------------------
int ectl_mem_set(int nb, uint16_t addr, uint16_t *src, int count)
{
	return mem_mput(nb, addr, src, count);
}

// -----------------------------------------------------------------------
int ectl_cpu_state_get()
{
	return 666;
}

// -----------------------------------------------------------------------
void ectl_cpu_state_set(int state)
{

}

// -----------------------------------------------------------------------
void ectl_cycle()
{

}

// -----------------------------------------------------------------------
void ectl_clock_set(int state)
{
	if (state == ECTL_OFF) {
		timer_off();
	} else {
		timer_on();
	}
}

// -----------------------------------------------------------------------
int ectl_clock_get()
{
	if (timer_get_state()) {
		return ECTL_ON;
	} else {
		return ECTL_OFF;
	}
}

// -----------------------------------------------------------------------
void ectl_clear()
{

}

// -----------------------------------------------------------------------
void ectl_bootstrap(int chan, int unit)
{

}

// -----------------------------------------------------------------------
void ectl_oprq()
{
	int_set(INT_OPRQ);
}

// vim: tabstop=4 shiftwidth=4 autoindent
