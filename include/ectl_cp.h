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

#include <inttypes.h>

#ifndef ECTL_CP_H
#define ECTL_CP_H

enum ectl_registers {
	ECTL_REG_R0 = 0,
	ECTL_REG_R1,
	ECTL_REG_R2,
	ECTL_REG_R3,
	ECTL_REG_R4,
	ECTL_REG_R5,
	ECTL_REG_R6,
	ECTL_REG_R7,
	ECTL_REG_IC,
	ECTL_REG_SR,
	ECTL_REG_IR,
	ECTL_REG_KB,
	ECTL_REG_MOD,
	ECTL_REG_MODc,
	ECTL_REG_ALARM,
	ECTL_REG_COUNT
};

const char * ectl_reg_name(unsigned n);
int ectl_reg_get_id(char *name);
void ectl_regs_get(uint16_t *dregs);
int ectl_reg_get(unsigned reg);
int ectl_reg_set(unsigned reg, uint16_t val);
int ectl_mem_get(int nb, uint16_t addr, uint16_t *dest, int count);
int ectl_mem_set(int nb, uint16_t addr, uint16_t *src, int count);
int ectl_mem_map(int nb);
int ectl_cpu_state_get();
void ectl_cpu_stop();
void ectl_cpu_start();
void ectl_cpu_cycle();
void ectl_cpu_quit();
void ectl_clock_set(int state);
int ectl_clock_get();
void ectl_cpu_clear();
void ectl_bootstrap(int chan, int unit);
void ectl_oprq();
int ectl_int_set(unsigned interrupt);
uint32_t ectl_int_get();

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
