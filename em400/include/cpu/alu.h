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

#include "cpu/registers.h"

void alu_add16(int reg, uint16_t arg, uint16_t carry);
void alu_add32(int reg1, int reg2, uint16_t arg1, uint16_t arg2, int sign);
void alu_mul32(int reg1, int reg2, int16_t arg);
void alu_div32(int reg1, int reg2, int16_t arg);
void alu_compare(int32_t a, int32_t b);
void alu_negate(int reg, uint16_t carry);

void alu_set_flags_LEG(int16_t x, int16_t y);
void alu_set_flag_Z(uint64_t z, int bits);
void alu_set_flag_M(uint64_t z, int bits);
void alu_set_flag_C(uint64_t z, int bits);
void alu_set_flag_V(uint64_t x, uint64_t y, uint64_t z, int bits);
void alu_set_flags_ZMVC(uint64_t x, uint64_t y, uint64_t z, int bits);

#define Fget(x)     ((reg_read(0, 1) & x) ? 1 : 0)
#define Fset(x)     reg_write(0, reg_read(0, 1) | x, 1, 1)
#define Fclr(x)     reg_write(0, reg_read(0, 1) & ~x, 1, 1)

#define DWORD(x, y)	(uint32_t) (x << 16) | (uint16_t) y
#define DWORDl(z)	(uint16_t) (z >> 16)
#define DWORDr(z)	(uint16_t) z

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
