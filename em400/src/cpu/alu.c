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
#include <fenv.h>

#include "cpu/alu.h"
#include "cpu/cpu.h"
#include "cpu/registers.h"
#include "cpu/interrupts.h"

// get bit n
#define BIT(n, z)	((z) & ((uint64_t) 1 << (n)))
// get bits n...0
#define BITS(n, z)	((z) & (((uint64_t) 1 << ((n)+1)) - 1))

int alu_32_V;

// -----------------------------------------------------------------------
// ---- 16-bit -----------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
void alu_16_add(unsigned reg, uint16_t arg, unsigned carry, int sign)
{
	if (sign < 0) {
		arg = -arg;
	}

	uint64_t res = regs[reg] + arg + carry;

	alu_16_set_V(regs[reg], arg+carry, res);
	alu_16_set_M(res);
	alu_16_set_C(res);
	alu_16_set_Z(res);

	// 0-0 always sets carry
	if ((sign < 0) && (regs[IR_A] == 0) && (N == 0)) {
		Fset(FL_C);
	}

	reg_safe_write(reg, (uint16_t) res);
}

// -----------------------------------------------------------------------
void alu_16_neg(int reg, uint16_t carry)
{
	uint16_t a = regs[reg];
	uint32_t res = (~a) + carry;

	alu_16_set_V(~a, carry, res);
	alu_16_set_M(res);
    // -0 sets carry
	if ((regs[IR_A] == 0) && (carry)) {
		Fset(FL_C);
	} else {
		Fclr(FL_C);
	}
	alu_16_set_Z(res);
	reg_safe_write(reg, res);
}

// -----------------------------------------------------------------------
void alu_16_set_LEG(int32_t a, int32_t b)
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
void alu_16_set_Z(uint64_t z)
{
	// for arithmetic zero, V and C needs to be set prior Z
	// set Z if:
	//  * all 16 bits of result are 0 and there is no carry
	//  * OR all 16 bits of result is 0 and carry is set, but overflow is not (case of -1+1)

	if (!BITS(15, z) && (!Fget(FL_C) || (Fget(FL_C) && !Fget(FL_V)))) {
		Fset(FL_Z);
	} else {
		Fclr(FL_Z);
	}
}

// -----------------------------------------------------------------------
void alu_16_set_M(uint64_t z)
{
	// V needs to be set prior M
	// set M if:
	//  * minus and no overflow (just a plain negative number)
	//  * OR not minus and overflow (number looks non-negative, but there is overflow, which means a negative number overflown)

	if ((BIT(15, z) && !Fget(FL_V)) || (!BIT(15, z) && Fget(FL_V))) {
		Fset(FL_M);
	} else {
		Fclr(FL_M);
	}
}

// -----------------------------------------------------------------------
void alu_16_set_C(uint64_t z)
{
	// set C if bit on position -1 is set

	if (BIT(16, z)) {
		Fset(FL_C);
	} else {
		Fclr(FL_C);
	}
}

// -----------------------------------------------------------------------
void alu_16_set_V(uint64_t x, uint64_t y, uint64_t z)
{
	// set V if:
	//  * both arguments were positive, and result is negative
	//  * OR both arguments were negative, and result is positive

	if ((BIT(15, x) && BIT(15, y) && !BIT(15, z)) || (!BIT(15, x) && !(BIT(15, y)) && BIT(15, z))) {
		Fset(FL_V);
	} else {
		Fclr(FL_V);
	}
}

// -----------------------------------------------------------------------
void alu_16_update_V(uint64_t x, uint64_t y, uint64_t z)
{
	// update V if it's not set AND:
	//  * both arguments were positive, and result is negative
	//  * OR both arguments were negative, and result is positive

	if ((BIT(15, x) && BIT(15, y) && !BIT(15, z)) || (!BIT(15, x) && !(BIT(15, y)) && BIT(15, z))) {
		Fset(FL_V);
	}
}

// -----------------------------------------------------------------------
// ---- 32-bit -----------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
void alu_32_add(uint16_t arg1, uint16_t arg2, int sign)
{
	uint32_t a1 = DWORD(regs[1], regs[2]);
	uint32_t a2 = DWORD(arg1, arg2);
	uint64_t res = (uint64_t) a1 + (sign * a2);
	regs[1] = DWORDl(res);
	regs[2] = DWORDr(res);

	alu_32_update_V(a1, sign*a2, res);
	alu_32_set_M(res);
	alu_32_set_C(res);
	alu_32_set_Z(res);
}

// -----------------------------------------------------------------------
void alu_32_mul(int16_t arg)
{
	int64_t res = (int16_t) regs[2] * arg;
	regs[1] = DWORDl(res);
	regs[2] = DWORDr(res);
	// alu_32_mul() has no effect on V
	alu_32_V = 0;
	alu_32_set_M(res);
	alu_32_set_Z(res);
}

// -----------------------------------------------------------------------
void alu_32_div(int16_t arg)
{
	if (!arg) {
		int_set(INT_DIV0);
		return;
	}
	int32_t d1 = DWORD(regs[1], regs[2]);
	int32_t res = d1 / arg;
	if ((res > 32767) || (res < -32768)) {
		int_set(INT_DIV_OF);
	} else {
		regs[2] = res;
		regs[1] = d1 % arg;
		// alu_32_div() has no effect on V
		alu_32_V = 0;
		// alu_32_div() has no effect on C
		alu_32_set_M(res);
		alu_32_set_Z(res);
	}
}

// -----------------------------------------------------------------------
void alu_32_set_Z(uint64_t z)
{
	// set Z if:
	//  * all 16 bits of result are 0

	if (!BITS(31, z)) {
		Fset(FL_Z);
	} else {
		Fclr(FL_Z);
	}
}

// -----------------------------------------------------------------------
void alu_32_update_V(uint64_t x, uint64_t y, uint64_t z)
{
	// update V if:
	//  * both arguments were positive, and result is negative
	//  * OR both arguments were negative, and result is positive

	if ((BIT(31, x) && BIT(31, y) && !BIT(31, z)) || (!BIT(31, x) && !(BIT(31, y)) && BIT(31, z))) {
		Fset(FL_V);
		alu_32_V = 1;
	} else {
		alu_32_V = 0;
	}
}

// -----------------------------------------------------------------------
void alu_32_set_C(uint64_t z)
{
	// set C if bit on position -1 is set

	if (BIT(32, z)) {
		Fset(FL_C);
	} else {
		Fclr(FL_C);
	}
}

// -----------------------------------------------------------------------
void alu_32_set_M(uint64_t z)
{
	// internal 32-bit V needs to be set prior M
	// set M if:
	//  * minus and no overflow (just a plain negative number)
	//  * OR not minus and overflow (number looks non-negative, but there is overflow, which means a negative number overflown)

	if ((BIT(31, z) && !alu_32_V) || (!BIT(31, z) && alu_32_V)) {
		Fset(FL_M);
	} else {
		Fclr(FL_M);
	}
}

// -----------------------------------------------------------------------
// ---- Floating Point ---------------------------------------------------
// -----------------------------------------------------------------------

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
			if ((m_f >= -0.5f) && (m_f < 0.5f)) {
				int_set(INT_DIV0);
				return -1;
			}
		}

		*f = ldexp(m_f, exp);
	}
	return 0;
}

// -----------------------------------------------------------------------
void alu_fp_store(double f, int round)
{
	// fetch flags as left by fp operation
	int fe_flags = fetestexcept(FE_OVERFLOW | FE_UNDERFLOW);

	// get m, exp
	int exp;
	double m = frexp(f, &exp);

	// scale m to 64-bit
	int64_t m_int = ldexp(m, FP_M_BITS-1);

	// 'normalized' for frexp() is (-1,-0.5], [0.5,1)
	// 'normalized' for AWP is [-1,-0.5), [0.5,1)
	// example: -1 + -1 = -2
	// 0x8000 0x0000 0x0000 + 0x8000 0x0000 0x0000 = 0x8000 0x0000 0x0001 (without fix it's: 0xc000 0x0000 0x0002)
	// so if result is -0.5 * 2^n make it -1 * 2^(n-1)
	if ((m_int != 0) && ((BIT(63, m_int) >> 1) == BIT(62, m_int))) {
		m_int <<= 1;
		exp--;
	}

	// AWP rounds up for add/sub if bit -1 of the result is set
	if (round && BIT(FP_M_BITS-1-40, m_int)) {
		// if we would overflow, need to shift first
		#define FULL_M_INT 0b0111111111111111111111111111111111111111000000000000000000000000
		if ((m_int & FULL_M_INT) == FULL_M_INT) {
			m_int >>= 1;
			exp++;
		}
		// add 1 on the least significant m_int position
		m_int += 1ULL << (FP_M_BITS-1-40);
	}

	// check host overflow/underflow, check if exponent fits in 8 bits
	if ((fe_flags & FE_OVERFLOW) || (exp > 127)) {
		int_set(INT_FP_OF);
	} else if ((fe_flags & FE_UNDERFLOW) || (exp < -128)) {
		int_set(INT_FP_UF);
	}

	uint16_t d1 = m_int >> 48;
	uint16_t d2 = m_int >> 32;
	uint16_t d3 = (m_int >> 16) & 0b1111111100000000;
	d3 |= exp & 255;

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

	regs[1] = d1;
	regs[2] = d2;
	regs[3] = d3;
}

// -----------------------------------------------------------------------
void alu_fp_norm()
{
	double f;
	if (!alu_fp_get(regs[1], regs[2], regs[3], &f, 0)) {
		Fclr(FL_C);
		feclearexcept(FE_ALL_EXCEPT);
		alu_fp_store(f, 0);
		Fclr(FL_C); // always 0?
	}
}

// -----------------------------------------------------------------------
void alu_fp_add(uint16_t d1, uint16_t d2, uint16_t d3, int sign)
{
	double f1, f2;
	if (!alu_fp_get(regs[1], regs[2], regs[3], &f1, 1)) {
		if (!alu_fp_get(d1, d2, d3, &f2, 1)) {
			feclearexcept(FE_ALL_EXCEPT);
			f2 *= sign;
			f1 += f2;
			alu_fp_store(f1, 1);
			// FP resets C in most cases... except one:
			// 0x6666 0x3333 0x33f7 - 0x6666 0x3333 0x33f0 = 0x6599 0x66cc 0xcdf7
			// don't know how to handle it, yet, so:
			Fclr(FL_C);
		}
	}
}

// -----------------------------------------------------------------------
void alu_fp_mul(uint16_t d1, uint16_t d2, uint16_t d3)
{
	double f1, f2;
	if (!alu_fp_get(regs[1], regs[2], regs[3], &f1, 1)) {
		if (!alu_fp_get(d1, d2, d3, &f2, 1)) {
			feclearexcept(FE_ALL_EXCEPT);
			f1 *= f2;
			alu_fp_store(f1, 0);
			Fclr(FL_C); // effectively always 0
		}
	}
}

// -----------------------------------------------------------------------
void alu_fp_div(uint16_t d1, uint16_t d2, uint16_t d3)
{
	double f1, f2;
	if (!alu_fp_get(regs[1], regs[2], regs[3], &f1, 1)) {
		if (!alu_fp_get(d1, d2, d3, &f2, 1)) {
			if (f2 == 0.0f) {
				int_set(INT_DIV0);
				return;
			} else {
				feclearexcept(FE_ALL_EXCEPT);
				f1 /= f2;
				alu_fp_store(f1, 0);
				Fclr(FL_C); // always 0
			}
		}
	}
}


// vim: tabstop=4 shiftwidth=4 autoindent
