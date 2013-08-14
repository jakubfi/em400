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
#include <math.h>

#include "cpu/alu.h"
#include "cpu/registers.h"
#include "cpu/interrupts.h"

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
void alu_add32(uint16_t arg1, uint16_t arg2, int sign)
{
	uint32_t a1 = DWORD(R(1), R(2));
	uint32_t a2 = DWORD(arg1, arg2);
	uint64_t res = (uint64_t) a1 + (sign * a2);
	alu_set_flag_Z(res, 32);
	alu_set_flag_M(res, 32);
	alu_set_flag_C(res, 32);
	alu_set_flag_V(a1, sign*a2, res, 32);
	Rw(1, DWORDl(res));
	Rw(2, DWORDr(res));
}

// -----------------------------------------------------------------------
void alu_mul32(int16_t arg)
{
	int64_t res = (int16_t) R(2) * arg;
	alu_set_flag_Z(res, 32);
	alu_set_flag_M(res, 32);
	Fclr(FL_V); // it seems that V is always 0 when doing mul32()
	Rw(1, DWORDl(res));
	Rw(2, DWORDr(res));
}

// -----------------------------------------------------------------------
void alu_div32(int16_t arg)
{
	if (!arg) {
		int_set(INT_DIV0);
		return;
	}
	int32_t d1 = DWORD(R(1), R(2));
	int32_t res = d1 / arg;
	if ((res > 32767) || (res < -32768)) {
		int_set(INT_DIV_OF);
	}
	alu_set_flag_Z(res, 32);
	alu_set_flag_M(res, 32);
	Rw(2, res);
	Rw(1, d1 % arg);
}

// -----------------------------------------------------------------------
int alu_fp_get(uint16_t d1, uint16_t d2, uint16_t d3, double *f, int check_norm)
{
	int64_t m;
	double m_f;
	int8_t exp;
	
	exp = d3 & 0b0000000011111111;
	m  = (int64_t) d1 << 48;
	m |= (int64_t) d2 << 32;
	m |= (int64_t) (d3 & 0b1111111100000000) << 16;

	if (m == 0) {
		m_f = 0.0f;
		*f = 0.0f;
	} else {
		m_f = ldexp(m, -(FP_M_BITS-1));

		// expecting normalized input
		if (check_norm) {
			if ((m_f <= -1) || (m_f >= 1) || ((m_f > -0.5) && (m_f < 0.5))) {
				int_set(INT_DIV0);
				return -1;
			}
		}

		*f = ldexp(m_f, exp);
	}
	//printf(" in: (0x%04x 0x%04x 0x%04x) = %.30f = %.30f * 2^%i (m=%li)\n", d1, d2, d3, *f, m_f, exp, m);
	return 0;
}

// -----------------------------------------------------------------------
void alu_fp_store(double f)
{
	// get m, exp
	int exp;
	double m = frexp(f, &exp);

	// scale m to 64-bit
	int64_t m_int = ldexp(m, FP_M_BITS-1);

	// check if exponent fits in 8 bits
	if (exp > 127) {
		int_set(INT_FP_OF);
	} else if (exp < -128) {
		int_set(INT_FP_UF);
	}

	uint16_t d1 = m_int >> 48;
	uint16_t d2 = m_int >> 32;
	uint16_t d3 = (m_int >> 16) & 0b1111111100000000;
	d3 |= exp & 255;

	// set C flag
	if (m_int & (1L << (FP_M_BITS-1-40))) {
		Fset(FL_C);
	} else {
		Fclr(FL_C);
	}

	// set Z and M (fp doesn't touch V)
	if (f == 0) {
		Fset(FL_Z);
		Fclr(FL_M);
	} else if (f < 0) {
		Fclr(FL_Z);
		Fset(FL_M);
	} else {
		Fclr(FL_Z);
		Fclr(FL_M);
	}

	Rw(1, d1);
	Rw(2, d2);
	Rw(3, d3);
	//printf("out: (0x%04x 0x%04x 0x%04x) = %.30f = %.30f * 2^%i (m=%li)\n", d1, d2, d3, f, m, exp, m_int);
}

// -----------------------------------------------------------------------
void alu_fp_norm()
{
	double f;
	if (!alu_fp_get(R(1), R(2), R(3), &f, 0)) {
		Fclr(FL_C);
		alu_fp_store(f);
	}
}

// -----------------------------------------------------------------------
void alu_fp_add(uint16_t d1, uint16_t d2, uint16_t d3, int sign)
{
	double f1, f2;
	if (!alu_fp_get(R(1), R(2), R(3), &f1, 1)) {
		if (!alu_fp_get(d1, d2, d3, &f2, 1)) {
			f2 *= sign;
			f1 += f2;
			alu_fp_store(f1);
		}
	}
}

// -----------------------------------------------------------------------
void alu_fp_mul(uint16_t d1, uint16_t d2, uint16_t d3)
{
	double f1, f2;
	if (!alu_fp_get(R(1), R(2), R(3), &f1, 1)) {
		if (!alu_fp_get(d1, d2, d3, &f2, 1)) {
			f1 *= f2;
			alu_fp_store(f1);
		}
	}
}

// -----------------------------------------------------------------------
void alu_fp_div(uint16_t d1, uint16_t d2, uint16_t d3)
{
	double f1, f2;
	if (!alu_fp_get(R(1), R(2), R(3), &f1, 1)) {
		if (!alu_fp_get(d1, d2, d3, &f2, 1)) {
			if (f2 == 0.0f) {
				int_set(INT_DIV0);
			} else {
				f1 /= f2;
				alu_fp_store(f1);
			}
		}
	}
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


// vim: tabstop=4 shiftwidth=4 autoindent
