//  Copyright (c) 2012-2013 Jakub Filipowicz <jakubf@gmail.com>
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

#ifndef ALU_H
#define ALU_H

#include <inttypes.h>

#include "registers.h"

uint32_t alu_add(uint32_t a, uint32_t b, uint32_t c, int bits);
void alu_compare(int32_t a, int32_t b);
uint16_t alu_negate(uint16_t a, uint16_t c);

void alu_set_flags_LEG(int16_t x, int16_t y);
void alu_set_flag_Z(uint64_t z, int bits);
void alu_set_flag_M(uint64_t z, int bits);
void alu_set_flag_C(uint64_t z, int bits);
void alu_set_flag_V(uint64_t x, uint64_t y, uint64_t z, int bits);
void alu_set_flags_ZMVC(uint64_t x, uint64_t y, uint64_t z, int bits);

#define Fget(x)     (reg_read(0, 1) & x) ? 1 : 0
#define Fset(x)     reg_write(0, reg_read(0, 1) | x, 1, 1)
#define Fclr(x)     reg_write(0, reg_read(0, 1) & ~x, 1, 1)

#endif

// vim: tabstop=4
