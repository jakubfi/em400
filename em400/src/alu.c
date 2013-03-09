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

#include <inttypes.h>

#include "alu.h"
#include "registers.h"
#include "interrupts.h"

// -----------------------------------------------------------------------
void alu_add16(int reg, uint16_t arg, uint16_t carry)
{
	uint32_t res = R(reg) + arg + carry;
    alu_set_flag_Z(res, 16);
    alu_set_flag_M(res, 16);
    alu_set_flag_C(res, 16);
    alu_set_flag_V(nR(reg), arg, res, 16);
	Rw(reg, (uint16_t) res);
}

// -----------------------------------------------------------------------
void alu_add32(int reg1, int reg2, uint16_t arg1, uint16_t arg2, int sign)
{
    uint32_t a1 = DWORD(R(reg1), R(reg2));
    uint32_t a2 = DWORD(arg1, arg2);
    uint64_t res = (uint64_t) a1 + (sign * a2);
	alu_set_flag_Z(res, 32);
	alu_set_flag_M(res, 32);
	alu_set_flag_C(res, 32);
	alu_set_flag_V(a1, sign*a2, res, 32);
    Rw(reg1, DWORDl(res));
    Rw(reg2, DWORDr(res));
}

// -----------------------------------------------------------------------
void alu_mul32(int reg1, int reg2, int16_t arg)
{
    int64_t res = (int16_t) R(reg2) * arg;
    alu_set_flag_Z(res, 32);
    alu_set_flag_M(res, 32);
	// TODO: overflow? how? why? we always have enough bits to store output
	Fclr(FL_V);
    Rw(reg1, DWORDl(res));
    Rw(reg2, DWORDr(res));
}

// -----------------------------------------------------------------------
void alu_div32(int reg1, int reg2, int16_t arg)
{
	if (!arg) {
		int_set(INT_DIV0);
		return;
	}
    int32_t d1 = DWORD(R(reg1), R(reg2));
    int32_t res = d1 / arg;
	if ((res > 32767) || (res < -32768)) {
		int_set(INT_FP_DIV_OF);
	}
    alu_set_flag_Z(res, 32);
    alu_set_flag_M(res, 32);
    Rw(reg2, res);
    Rw(reg1, d1 % arg);
}

// -----------------------------------------------------------------------
void alu_compare(int32_t a, int32_t b)
{
    if (a == b) {
        Fset(FL_E);
        Fclr(FL_G);
        Fclr(FL_L);
    } else {
        Fclr(FL_E);
        if (a < b) {
            Fset(FL_L);
            Fclr(FL_G);
        } else {
            Fclr(FL_L);
            Fset(FL_G);
        }
    }
}

// -----------------------------------------------------------------------
void alu_negate(int reg, uint16_t carry)
{
	uint16_t a = R(reg);
	uint32_t res = (uint16_t) (~a) + carry;
	alu_set_flag_Z(res, 16);
	alu_set_flag_M(res, 16);
	alu_set_flag_C(res, 16);
	alu_set_flag_V(a, a, res, 16);
	Rw(reg, res);
}

// -----------------------------------------------------------------------
void alu_set_flag_Z(uint64_t z, int bits)
{
    uint64_t mask_Z = ((uint64_t)1 << (bits))-1;
    if (z & mask_Z) {
        Fclr(FL_Z);
    } else {
        Fset(FL_Z);
    }
}

// -----------------------------------------------------------------------
void alu_set_flag_M(uint64_t z, int bits)
{
    uint64_t mask_M = (uint64_t) 1 << (bits-1);
    if (z & mask_M) {
        Fset(FL_M);
    } else {
        Fclr(FL_M);
    }
}

// -----------------------------------------------------------------------
void alu_set_flag_C(uint64_t z, int bits)
{
    uint64_t mask_C = (uint64_t) 1 << bits;
    if (z & mask_C) {
        Fset(FL_C);
    } else {
        Fclr(FL_C);
    }
}

// -----------------------------------------------------------------------
void alu_set_flag_V(uint64_t x, uint64_t y, uint64_t z, int bits)
{
    uint64_t mask_M = (uint64_t) 1 << (bits-1);
    if (((x & mask_M) && (y & mask_M) && !(z & mask_M)) || (!(x & mask_M) && !(y & mask_M) && (z & mask_M))) {
        Fset(FL_V);
    } else {
        Fclr(FL_V);
    }
}


// vim: tabstop=4
