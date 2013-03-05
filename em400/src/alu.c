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

// -----------------------------------------------------------------------
uint32_t alu_add(uint32_t a, uint32_t b, uint32_t c, int bits)
{
	uint64_t res = a + b + c;
    alu_set_flag_Z(res, bits);
    alu_set_flag_M(res, bits);
    alu_set_flag_C(res, bits);
    alu_set_flag_V(a, b, res, bits);
	return res;
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
void alu_set_flag_Z(uint64_t z, int bits)
{
    int64_t mask_Z = (1 << (bits))-1;
    if (z & mask_Z) {
        Fclr(FL_Z);
    } else {
        Fset(FL_Z);
    }
}

// -----------------------------------------------------------------------
void alu_set_flag_M(uint64_t z, int bits)
{
    int64_t mask_M = 1 << (bits-1);
    if (z & mask_M) {
        Fset(FL_M);
    } else {
        Fclr(FL_M);
    }
}

// -----------------------------------------------------------------------
void alu_set_flag_C(uint64_t z, int bits)
{
    int64_t mask_C = 1 << bits;
    if (z & mask_C) {
        Fset(FL_C);
    } else {
        Fclr(FL_C);
    }
}

// -----------------------------------------------------------------------
void alu_set_flag_V(uint64_t x, uint64_t y, uint64_t z, int bits)
{
    int64_t mask_M = 1 << (bits-1);
    if (((x & mask_M) && (y & mask_M) && !(z & mask_M)) || (!(x & mask_M) && !(y & mask_M) && (z & mask_M))) {
        Fset(FL_V);
    } else {
        Fclr(FL_V);
    }
}


// vim: tabstop=4
