//  Copyright (c) 2012-2015 Jakub Filipowicz <jakubf@gmail.com>
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

#include <stdlib.h>
#include <inttypes.h>
#include <math.h>
#include <fenv.h>
#include <limits.h>

#include "emawp.h"

#define FL_Z 0b1000000000000000
#define FL_M 0b0100000000000000
#define FL_V 0b0010000000000000
#define FL_C 0b0001000000000000

#define FL_SET(flags, flag) (flags) |= (flag)
#define FL_CLR(flags, flag) (flags) &= ~(flag)

#define DWORD(x, y) (uint32_t) ((x) << 16) | (uint16_t) (y)
#define DWORD_H(z)  (uint16_t) ((z) >> 16)
#define DWORD_L(z)  (uint16_t) (z)

// get bit n (n...0 bit numbering)
#define BIT(n, z)  ((z) & (1ULL << (n)))
// get bits n...0 (n...0 bit numbering)
#define BITS(n, z) ((z) & ((1ULL << ((n)+1)) - 1))

#define FP_BITS 64
#define FP_M_MASK 0b1111111111111111111111111111111111111111000000000000000000000000
#define FP_M_MAX  0b0111111111111111111111111111111111111111000000000000000000000000
#define FP_CORRECTION (1ULL << (FP_BITS-1-39))
// get nth bit of 40-bit mantissa (stored in 64-bit uint, 0...n bit numbering)
#define FP_BIT(n, z) ((z) & (1ULL << (FP_BITS-1-n)))


// -----------------------------------------------------------------------
struct awp * awp_init()
{
	struct awp * awp = calloc(1, sizeof(struct awp));
	if (!awp) return NULL;

	return awp;
}

// -----------------------------------------------------------------------
void awp_destroy(struct awp *awp)
{
	free(awp);
}

// -----------------------------------------------------------------------
void awp_load(struct awp *awp, uint16_t flags, uint16_t r1, uint16_t r2, uint16_t r3)
{
	awp->flags = flags;
	awp->r1 = r1;
	awp->r2 = r2;
	awp->r3 = r3;
}

// -----------------------------------------------------------------------
void awp_store(struct awp *awp, uint16_t *flags, uint16_t *r1, uint16_t *r2, uint16_t *r3)
{
	*flags = awp->flags;
	*r1 = awp->r1;
	*r2 = awp->r2;
	*r3 = awp->r3;
}

// -----------------------------------------------------------------------
// ---- 32-bit -----------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
static void __awp_dword_set_Z(struct awp *awp, uint64_t z)
{
	// set Z if:
	//  * all 32 bits of result are 0

	if (!BITS(31, z)) {
		FL_SET(awp->flags, FL_Z);
	} else {
		FL_CLR(awp->flags, FL_Z);
	}
}

// -----------------------------------------------------------------------
static void __awp_dword_update_V(struct awp *awp, uint64_t x, uint64_t y, uint64_t z)
{
	// update (not set!) V if:
	//  * both arguments were positive, and result is negative
	//  * OR both arguments were negative, and result is positive
	// store awp->v, M needs it

	if ((BIT(31, x) && BIT(31, y) && !BIT(31, z)) || (!BIT(31, x) && !(BIT(31, y)) && BIT(31, z))) {
		FL_SET(awp->flags, FL_V);
		awp->v = 1;
	} else {
		awp->v = 0;
	}
}

// -----------------------------------------------------------------------
static void __awp_dword_set_C(struct awp *awp, uint64_t z)
{
	// set C if bit on position -1 is set

	if (BIT(32, z)) {
		FL_SET(awp->flags, FL_C);
	} else {
		FL_CLR(awp->flags, FL_C);
	}
}

// -----------------------------------------------------------------------
static void __awp_dword_set_M(struct awp *awp, uint64_t z)
{
	// note: internal V needs to be set before setting M
	// set M if:
	//  * minus and no overflow (just a plain negative number)
	//  * OR not minus and overflow (number looks non-negative, but there is overflow, which means a negative number overflown)

	if ((BIT(31, z) && !awp->v) || (!BIT(31, z) && awp->v)) {
		FL_SET(awp->flags, FL_M);
	} else {
		FL_CLR(awp->flags, FL_M);
	}
}

// -----------------------------------------------------------------------
int awp_dword_add(struct awp *awp, uint16_t b1, uint16_t b2, int op)
{
	uint32_t a = DWORD(awp->r1, awp->r2);
	uint32_t b = DWORD(b1, b2);
	uint64_t res = (uint64_t) a + (op * b);

	awp->r1 = DWORD_H(res);
	awp->r2 = DWORD_L(res);

	// AD and SD set all flags

	__awp_dword_update_V(awp, a, op*b, res);
	__awp_dword_set_M(awp, res);
	__awp_dword_set_C(awp, res);
	__awp_dword_set_Z(awp, res);

	return AWP_OK;
}

// -----------------------------------------------------------------------
int awp_dword_mul(struct awp *awp, int16_t b)
{
	int64_t res = (int16_t) awp->r2 * b;

	awp->r1 = DWORD_H(res);
	awp->r2 = DWORD_L(res);

	// MW does not touch C
	// MW may touch V, but it won't (?)

	awp->v = 0; // internal only
	__awp_dword_set_M(awp, res);
	__awp_dword_set_Z(awp, res);

	return AWP_OK;
}

// -----------------------------------------------------------------------
int awp_dword_div(struct awp *awp, int16_t b)
{
	// NOTE: in case of AWP_FP_ERR CPU registers stay unchanged
	if (b == 0) {
		return AWP_FP_ERR;
	}

	int32_t a = DWORD(awp->r1, awp->r2);

	int32_t res = a / b;

	// NOTE: in case of AWP_DIV_OF CPU registers stay unchanged
	if ((res > SHRT_MAX) || (res < SHRT_MIN)) {
		return AWP_DIV_OF;
	}

	awp->r2 = res;
	awp->r1 = a % b;

	// DW has does not touch V nor C

	awp->v = 0; // internal only
	__awp_dword_set_M(awp, res);
	__awp_dword_set_Z(awp, res);

	return AWP_OK;
}

// -----------------------------------------------------------------------
// ---- Floating Point ---------------------------------------------------
// -----------------------------------------------------------------------

// NOTE: normalized floating-point 0 has all 48 bits set to 0
// NOTE: no floating-point operation touches V
// NOTE: in case of denormalized input (except denormalized 0)
//       or division by 0 no CPU registers are updated
// NOTE: in case of underflow or overflow CPU registers are updated anyway

// -----------------------------------------------------------------------
static int awp_float_decode(double *f, uint16_t d1, uint16_t d2, uint16_t d3)
{
	int64_t m;
	double m_f;
	int8_t exp;
	int ret = AWP_OK;

	exp = d3 & 0x00ff;
	m  = (int64_t) d1 << 48;
	m |= (int64_t) d2 << 32;
	m |= (int64_t) (d3 & 0xff00) << 16;

	if (m == 0) {
		*f = 0.0f;
	} else {
		// indicate that input is denormalized
		if ((FP_BIT(0, m) >> 1) == FP_BIT(1, m)) {
			ret = AWP_FP_ERR;
		}

		// convert mantissa to floating point: m_f = m * 2^-(FP_BITS-1)
		m_f = ldexp(m, -(FP_BITS-1));

		// make final fp number: f = m_f * 2^exp
		*f = ldexp(m_f, exp);
	}

	return ret;
}

// -----------------------------------------------------------------------
static int awp_float_encode(uint16_t *d1, uint16_t *d2, uint16_t *d3, uint16_t *flags, double f, int af_sf)
{
	int ret = AWP_OK;

	int exp;
	int64_t m_int;
	double m;

	// get m, exp
	m = frexp(f, &exp);
	// scale m to 64-bit
	m_int = ldexp(m, FP_BITS-1);

/*
	Normalize

	normalized mantissa for frexp() is: (-1, -0.5], [0.5, 1)
	normalized mantissa for AWP is:     [-1, -0.5), [0.5, 1)
	
	example for -1 + -1 = -2
	
	-1  0x8000 0x0000 0x0000 +
	-1  0x8000 0x0000 0x0000
	=   0xc000 0x0000 0x0002 = -0.5 * 2^n     (without the fix)
	=   0x8000 0x0000 0x0001 = -1   * 2^(n-1) (with the fix)
*/
	if ((m_int != 0) && ((FP_BIT(0, m_int) >> 1) == FP_BIT(1, m_int))) {
		m_int <<= 1;
		exp--;
	}

	// AWP rounds up for AF/SF if bit 40 (stored in M[-1]) of the result is set
	if (af_sf && FP_BIT(40, m_int)) {
		// if we would overflow, need to shift right first
		if ((m_int & FP_M_MAX) == FP_M_MAX) {
			m_int >>= 1;
			exp++;
			m_int += FP_CORRECTION >> 1;
		} else {
			m_int += FP_CORRECTION;
		}
	}

	// check for overflow/underflow
	if (exp > 127) {
		ret = AWP_FP_OF;
	} else if ((exp < -128) && (m_int != 0)) {
		ret = AWP_FP_UF;
	}

	*d1 = m_int >> 48;
	*d2 = m_int >> 32;
	*d3 = (m_int >> 16) & 0xff00;
	*d3 |= exp & 0xff;

	// fp operations don't touch V
	// set Z and M
	if ((m_int & FP_M_MASK) == 0) {
		FL_SET(*flags, FL_Z);
		FL_CLR(*flags, FL_M);
	} else if (FP_BIT(0, m_int)) {
		FL_CLR(*flags, FL_Z);
		FL_SET(*flags, FL_M);
	} else {
		FL_CLR(*flags, FL_Z);
		FL_CLR(*flags, FL_M);
	}

	// set C to M[-1] (used only by AF/SF)
	if (af_sf && FP_BIT(40, m_int)) {
		FL_SET(*flags, FL_C);
	} else {
		FL_CLR(*flags, FL_C);
	}

	return ret;
}

// -----------------------------------------------------------------------
int awp_float_norm(struct awp *awp)
{
	int res;
	double f;

	// ignore return value because denormalized input is what we expect here
	awp_float_decode(&f, awp->r1, awp->r2, awp->r3);
	res = awp_float_encode(&awp->r1, &awp->r2, &awp->r3, &awp->flags, f, 0);

	// NOTE: normalization always clears C
	FL_CLR(awp->flags, FL_C);

	return res;
}

// -----------------------------------------------------------------------
int awp_float_add(struct awp *awp, uint16_t d1, uint16_t d2, uint16_t d3, int op)
{
	int res;
	double f1, f2;

	res = awp_float_decode(&f1, awp->r1, awp->r2, awp->r3);
	if (res != AWP_OK) {
		return res;
	}

	res = awp_float_decode(&f2, d1, d2, d3);
	if (res != AWP_OK) {
		return res;
	}

	f1 += op * f2;

	// NOTE: AF and SF rounds up the result
	return awp_float_encode(&awp->r1, &awp->r2, &awp->r3, &awp->flags, f1, 1);
}

// -----------------------------------------------------------------------
int awp_float_mul(struct awp *awp, uint16_t d1, uint16_t d2, uint16_t d3)
{
	int res;
	double f1, f2;

	res = awp_float_decode(&f1, awp->r1, awp->r2, awp->r3);
	if (res != AWP_OK) {
		return res;
	}

	res = awp_float_decode(&f2, d1, d2, d3);
	if (res != AWP_OK) {
		return res;
	}

	f1 *= f2;
	res = awp_float_encode(&awp->r1, &awp->r2, &awp->r3, &awp->flags, f1, 0);

	FL_CLR(awp->flags, FL_C); // effectively always 0

	return res;
}

// -----------------------------------------------------------------------
int awp_float_div(struct awp *awp, uint16_t d1, uint16_t d2, uint16_t d3)
{
	int res;
	double f1, f2;

	res = awp_float_decode(&f1, awp->r1, awp->r2, awp->r3);
	if (res != AWP_OK) {
		return res;
	}

	res = awp_float_decode(&f2, d1, d2, d3);
	if (res != AWP_OK) {
		return res;
	}

	if (f2 == 0.0f) {
		return AWP_FP_ERR;
	}

	f1 /= f2;
	res = awp_float_encode(&awp->r1, &awp->r2, &awp->r3, &awp->flags, f1, 0);

	// NOTE: DF always clears C
	FL_CLR(awp->flags, FL_C);

	return res;
}

// -----------------------------------------------------------------------
int awp_to_double(double *f, uint16_t d1, uint16_t d2, uint16_t d3)
{
	return awp_float_decode(f, d1, d2, d3);
}

// -----------------------------------------------------------------------
int awp_from_double(uint16_t *d1, uint16_t *d2, uint16_t *d3, uint16_t *flags, double f)
{
	return awp_float_encode(d1, d2, d3, flags, f, 0);
}

// vim: tabstop=4 shiftwidth=4 autoindent
