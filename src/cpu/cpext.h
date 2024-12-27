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

#ifndef __CPEXT_H__
#define __CPEXT_H__

#include <stdint.h>
#include <stdbool.h>

int cpext_reg(unsigned reg_id);
void cpext_regs(uint16_t *dest);
void cpext_reg_set(unsigned reg_id, uint16_t val);
unsigned cpext_nb();
unsigned cpext_qnb();
uint32_t cpext_rz32();
int cpext_int_set(unsigned interrupt);
int cpext_int_clear(unsigned interrupt);

unsigned int cpext_state();
bool cpext_mem_read_n(unsigned nb, uint16_t addr, uint16_t *data, unsigned count);
bool cpext_mem_write_n(unsigned nb, uint16_t addr, uint16_t *data, unsigned count);
int cpext_mem_get_map(unsigned seg);


#endif

// vim: tabstop=4 shiftwidth=4 autoindent
