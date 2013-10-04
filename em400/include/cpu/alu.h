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

#include "cpu/cpu.h"

#define FP_M_BITS 64

void alu_16_add(unsigned reg, uint16_t arg, unsigned carry, int sign);
void alu_16_neg(int reg, uint16_t carry);
void alu_16_set_LEG(int32_t a, int32_t b);
void alu_16_set_Z(uint64_t z);
void alu_16_set_M(uint64_t z);
void alu_16_set_C(uint64_t z);
void alu_16_set_V(uint64_t x, uint64_t y, uint64_t z);
void alu_16_update_V(uint64_t x, uint64_t y, uint64_t z);

void alu_32_add(uint16_t arg1, uint16_t arg2, int sign);
void alu_32_mul(int16_t arg);
void alu_32_div(int16_t arg);
void alu_32_set_Z(uint64_t z);
void alu_32_update_V(uint64_t x, uint64_t y, uint64_t z);
void alu_32_set_C(uint64_t z);
void alu_32_set_M(uint64_t z);

void alu_fp_norm();
void alu_fp_add(uint16_t d1, uint16_t d2, uint16_t d3, int sign);
void alu_fp_mul(uint16_t d1, uint16_t d2, uint16_t d3);
void alu_fp_div(uint16_t d1, uint16_t d2, uint16_t d3);

#define Fget(x)     (regs[0] & (x) ? 1 : 0)
#define Fset(x)     regs[0] = regs[0] | (x)
#define Fclr(x)     regs[0] = regs[0] & ~(x)

#define DWORD(x, y)	(uint32_t) ((x) << 16) | (uint16_t) (y)
#define DWORDl(z)	(uint16_t) ((z) >> 16)
#define DWORDr(z)	(uint16_t) (z)

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
