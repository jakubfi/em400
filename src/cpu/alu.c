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
#include <limits.h>

#include "cfg.h"
#include "mem/mem.h"
#include "cpu/alu.h"
#include "cpu/emawp.h"
#include "cpu/cpu.h"
#include "cpu/registers.h"
#include "cpu/interrupts.h"

#include "log.h"

int awp_32_V;

#define AWP_DISPATCH_TAB_ADDR 100

#define BIT(n, z)   ((z) & (1ULL << (n)))           // get bit n (n...0 bit numbering)
#define BITS(n, z)  ((z) & ((1ULL << ((n)+1)) - 1)) // get bits n...0 (n...0 bit numbering)

#define FP_BITS 64
#define FP_M_MASK 0b1111111111111111111111111111111111111111000000000000000000000000
#define FP_M_MAX 0b0111111111111111111111111111111111111111000000000000000000000000
#define FP_CORRECTION (1ULL << (FP_BITS-1-39))
#define FP_BIT(n, z) ((z) & (1ULL << (FP_BITS-1-n))) // get nth bit of 40-bit mantissa (stored in 64-bit uint, 0...n bit numbering)

#define DWORD(x, y) (uint32_t) ((x) << 16) | (uint16_t) (y)
#define DWORDl(z)   (uint16_t) ((z) >> 16)
#define DWORDr(z)   (uint16_t) (z)

void awp_32_set_Z(uint64_t z);
void awp_32_update_V(uint64_t x, uint64_t y, uint64_t z);
void awp_32_set_C(uint64_t z);
void awp_32_set_M(uint64_t z);

void awp_fp_norm();
void awp_fp_add(uint16_t d1, uint16_t d2, uint16_t d3, int sign);
void awp_fp_mul(uint16_t d1, uint16_t d2, uint16_t d3);
void awp_fp_div(uint16_t d1, uint16_t d2, uint16_t d3);

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

	reg_restrict_write(reg, (uint16_t) res);
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
	reg_restrict_write(reg, res);
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
// ---- AWP --------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
void awp_dispatch(int op, uint16_t arg)
{
	int res;
	uint16_t d[3];
	uint16_t addr;

	if (cpu_awp) {
		if (!mem_cpu_mget(QNB, arg, d, 3)) return;

		awp_load(awp, regs[0], regs[1], regs[2], regs[3]);

		switch (op) {
			case AWP_NRF0:
			case AWP_NRF1:
			case AWP_NRF2:
			case AWP_NRF3:
				res = awp_float_norm(awp);
				break;
			case AWP_AD:
				res = awp_dword_add(awp, d[0], d[1], OP_ADD);
				break;
			case AWP_SD:
				res = awp_dword_add(awp, d[0], d[1], OP_SUB);
				break;
			case AWP_MW:
				res = awp_dword_mul(awp, d[0]);
				break;
			case AWP_DW:
				res = awp_dword_div(awp, d[0]);
				break;
			case AWP_AF:
				res = awp_float_add(awp, d[0], d[1], d[2], OP_ADD);
				break;
			case AWP_SF:
				res = awp_float_add(awp, d[0], d[1], d[2], OP_SUB);
				break;
			case AWP_MF:
				res = awp_float_mul(awp, d[0], d[1], d[2]);
				break;
			case AWP_DF:
				res = awp_float_div(awp, d[0], d[1], d[2]);
				break;
		}

		if (res < AWP_CRITICAL) {
			awp_store(awp, regs+0, regs+1, regs+2, regs+3);
		}

		switch (res) {
			case AWP_FP_UF:
				int_set(INT_FP_UF);
				break;
			case AWP_FP_OF:
				int_set(INT_FP_OF);
				break;
			case AWP_DIV_OF:
				int_set(INT_DIV_OF);
				break;
			case AWP_FP_ERR:
				int_set(INT_FP_ERR);
				break;
		}
	} else {
		if (!mem_cpu_get(0, AWP_DISPATCH_TAB_ADDR+op, &addr)) return;
		if (!cpu_ctx_switch(arg, addr, MASK_9 & MASK_Q)) return;
	}
}


// vim: tabstop=4 shiftwidth=4 autoindent
