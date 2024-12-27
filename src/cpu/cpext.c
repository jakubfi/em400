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

#include <stdint.h>
#include <stdbool.h>

#include "cpu/cpu.h"
#include "cpu/interrupts.h"
#include "mem/mem.h"

#include "libem400.h"

// -----------------------------------------------------------------------
int cpext_reg(unsigned reg_id)
{
	return cpu_reg_fetch(reg_id);
}

// -----------------------------------------------------------------------
void cpext_regs(uint16_t *dest)
{
	for (int i=0 ; i<EM400_REG_COUNT ; i++) {
		dest[i] = cpu_reg_fetch(i);
	}
}

// -----------------------------------------------------------------------
void cpext_reg_set(unsigned reg_id, uint16_t val)
{
	cpu_reg_load(reg_id, val);
}

// -----------------------------------------------------------------------
unsigned cpext_nb()
{
	return nb;
}

// -----------------------------------------------------------------------
unsigned cpext_qnb()
{
	return q * nb;
}

// -----------------------------------------------------------------------
uint32_t cpext_rz32()
{
	return rz;
}

// -----------------------------------------------------------------------
int cpext_int_set(unsigned interrupt)
{
	if (interrupt >= 32) {
		return -1;
	}
	int_set(interrupt);
	return 0;
}

// -----------------------------------------------------------------------
int cpext_int_clear(unsigned interrupt)
{
	if (interrupt >= 32) {
		return -1;
	}
	int_clear(interrupt);
	return 0;
}

// -----------------------------------------------------------------------
unsigned cpext_state()
{
	return cpu_state_get();
}

// -----------------------------------------------------------------------
bool cpext_mem_read_n(unsigned nb, uint16_t addr, uint16_t *data, unsigned count)
{
	return mem_read_n(nb, addr, data, count);
}

// -----------------------------------------------------------------------
bool cpext_mem_write_n(unsigned nb, uint16_t addr, uint16_t *data, unsigned count)
{
	return mem_write_n(nb, addr, data, count);
}

// -----------------------------------------------------------------------
int cpext_mem_get_map(unsigned seg)
{
	return mem_get_map(seg);
}

// vim: tabstop=4 shiftwidth=4 autoindent
