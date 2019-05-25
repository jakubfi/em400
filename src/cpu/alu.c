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

#include <emawp.h>

#include "cfg.h"
#include "mem/mem.h"
#include "cpu/alu.h"
#include "cpu/cpu.h"
#include "cpu/interrupts.h"

#include "log.h"

#define AWP_DISPATCH_TAB_ADDR 100

// all constant names use MERA-400 0...n bit numbering

#define BIT_0		0x08000
#define BIT_MINUS_1 0x10000
#define BITS_0_15	0x0ffff

// -----------------------------------------------------------------------
// ---- 16-bit -----------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
void alu_16_set_LEG(int32_t a, int32_t b)
{
	if (a == b) {
		Fset(FL_E);
		Fclr(FL_G | FL_L);
	} else {
		if (a < b) {
			Fset(FL_L);
			Fclr(FL_E | FL_G);
		} else {
			Fclr(FL_E | FL_L);
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

	if (!(z & BITS_0_15) && (!Fget(FL_C) || (Fget(FL_C) && !Fget(FL_V)))) {
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

	if (((z & BIT_0) && !Fget(FL_V)) || (!(z & BIT_0) && Fget(FL_V))) {
		Fset(FL_M);
	} else {
		Fclr(FL_M);
	}
}

// -----------------------------------------------------------------------
void alu_16_set_C(uint64_t z)
{
	// set C if bit on position -1 is set

	if ((z & BIT_MINUS_1)) {
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

	if (
	   ( (x & BIT_0) &&  (y & BIT_0) && !(z & BIT_0))
	|| (!(x & BIT_0) && !(y & BIT_0) &&  (z & BIT_0))
	) {
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

	if (
	   ( (x & BIT_0) &&  (y & BIT_0) && !(z & BIT_0))
	|| (!(x & BIT_0) && !(y & BIT_0) &&  (z & BIT_0))
	) {
		Fset(FL_V);
	}
}

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
// ---- AWP --------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
void awp_dispatch(int op, uint16_t arg)
{
	int res = 0;
	uint16_t d[3];
	uint16_t addr;

	if (awp) {
		if (cpu_mem_mget(QNB, arg, d, 3) != 3) return;

		switch (op) {
			case AWP_NRF0:
			case AWP_NRF1:
			case AWP_NRF2:
			case AWP_NRF3:
				res = awp_float_norm(awp);
				break;
			case AWP_AD:
				res = awp_dword_addsub(awp, d[0], d[1], AWP_OP_ADD);
				break;
			case AWP_SD:
				res = awp_dword_addsub(awp, d[0], d[1], AWP_OP_SUB);
				break;
			case AWP_MW:
				res = awp_dword_mul(awp, d[0]);
				break;
			case AWP_DW:
				res = awp_dword_div(awp, d[0]);
				break;
			case AWP_AF:
				res = awp_float_addsub(awp, d[0], d[1], d[2], AWP_OP_ADD);
				break;
			case AWP_SF:
				res = awp_float_addsub(awp, d[0], d[1], d[2], AWP_OP_SUB);
				break;
			case AWP_MF:
				res = awp_float_mul(awp, d[0], d[1], d[2]);
				break;
			case AWP_DF:
				res = awp_float_div(awp, d[0], d[1], d[2]);
				break;
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
		if (!cpu_mem_get(0, AWP_DISPATCH_TAB_ADDR+op, &addr)) return;
		if (!cpu_ctx_switch(arg, addr, MASK_9)) return;
	}
}

// vim: tabstop=4 shiftwidth=4 autoindent
