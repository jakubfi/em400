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

enum alu_awp_ops_e {
	AWP_NRF0 = 0, AWP_NRF1 = 1, AWP_NRF2 = 2, AWP_NRF3 = 3, // NRFs need to be 0-3
	AWP_AD, AWP_SD, AWP_MW, AWP_DW,
	AWP_AF, AWP_SF, AWP_MF, AWP_DF,
};

void alu_16_add(unsigned reg, uint16_t arg, unsigned carry, int sign);
void alu_16_neg(int reg, uint16_t carry);
void alu_16_set_LEG(int32_t a, int32_t b);
void alu_16_set_Z(uint64_t z);
void alu_16_set_M(uint64_t z);
void alu_16_set_C(uint64_t z);
void alu_16_set_V(uint64_t x, uint64_t y, uint64_t z);
void alu_16_update_V(uint64_t x, uint64_t y, uint64_t z);

void awp_dispatch(int op, uint16_t arg);

void awp_32_add(uint16_t arg1, uint16_t arg2, int sign);
void awp_32_mul(int16_t arg);
void awp_32_div(int16_t arg);

#define Fget(x) (regs[0] & (x) ? 1 : 0)
#define Fset(x) regs[0] |= (x)
#define Fclr(x) regs[0] &= ~(x)

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
