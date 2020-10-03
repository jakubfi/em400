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
#include <assert.h>
#include <emawp.h>

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
void alu_16_set_Z_bool(uint16_t z)
{
	if (z == 0) Fset(FL_Z);
	else Fclr(FL_Z);
}

// -----------------------------------------------------------------------
void alu_16_add(int16_t r, int16_t n, unsigned carry)
{
	int sres = r + n + carry;
	unsigned ures = (uint16_t) r + (uint16_t) n + carry;

	if (sres == 0) {
		Fset(FL_Z);
	} else {
		Fclr(FL_Z);
	}

	if (ures & BIT_MINUS_1) {
		Fset(FL_C);
	} else {
		Fclr(FL_C);
	}

	// Straight from the schematic:
	// Set M when one of the following is true:
	//  * C=1 and n[0] == r[0]
	//  * C=0 and n[0] != r[0]
	int diff_bit_zero = ((r^n) & BIT_0) >> 15;
	if (Fget(FL_C) != diff_bit_zero) {
		Fset(FL_M);
	} else {
		Fclr(FL_M);
	}

	// NOTE: straight from the schematic
	if (((ures & BIT_0) >> 15) ^ Fget(FL_M)) {
		Fset(FL_V);
	} else {
		// V is never cleared
	}

	reg_restrict_write(IR_A, (uint16_t) ures);
}


// -----------------------------------------------------------------------
void alu_16_sub(int16_t r, int16_t n)
{
	int sres = r - n;
	unsigned ures = (uint16_t) r + (uint16_t) -n;

	if (sres == 0) {
		Fset(FL_Z);
	} else {
		Fclr(FL_Z);
	}

	// NOTE: x-0 always sets carry
	if ((ures & BIT_MINUS_1) || (n == 0)) {
		Fset(FL_C);
	} else {
		Fclr(FL_C);
	}

	// straight from the schematic
	// Set M when one of the following is true:
	//  * C=1 and n[0] != r[0]
	//  * C=0 and n[0] == r[0]
	int diff_bit_zero = ((r^n) & BIT_0) >> 15;
	if (Fget(FL_C) == diff_bit_zero) {
		Fset(FL_M);
	} else {
		Fclr(FL_M);
	}

	// NOTE: straight from the schematic
	if (((ures & BIT_0) >> 15) ^ Fget(FL_M)) {
		Fset(FL_V);
	} else {
		// V is never cleared
	}
	reg_restrict_write(IR_A, (uint16_t) ures);
}

// -----------------------------------------------------------------------
// ---- AWP --------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
void awp_dispatch(int op, uint16_t arg)
{
	int res = 0;
	uint16_t n[3];
	uint16_t addr;

	assert(op >= AWP_NRF0);
	assert(op <= AWP_DF);

	static const int awp_arg_count[] = {
		1, // AWP_NRF0
		1, // AWP_NRF1
		1, // AWP_NRF2
		1, // AWP_NRF3
		2, // AWP_AD
		2, // AWP_SD
		1, // AWP_MW
		1, // AWP_DW
		3, // AWP_AF
		3, // AWP_SF
		3, // AWP_MF
		3, // AWP_DF
	};

	if (awp_enabled) {
		if (cpu_mem_mget(QNB, arg, n, awp_arg_count[op]) != awp_arg_count[op]) return;

		switch (op) {
			case AWP_NRF0:
			case AWP_NRF1:
			case AWP_NRF2:
			case AWP_NRF3:
				res = awp_float_norm(regs);
				break;
			case AWP_AD:
				res = awp_dword_addsub(regs, n, AWP_OP_ADD);
				break;
			case AWP_SD:
				res = awp_dword_addsub(regs, n, AWP_OP_SUB);
				break;
			case AWP_MW:
				res = awp_dword_mul(regs, n[0]);
				break;
			case AWP_DW:
				res = awp_dword_div(regs, n[0]);
				break;
			case AWP_AF:
				res = awp_float_addsub(regs, n, AWP_OP_ADD);
				break;
			case AWP_SF:
				res = awp_float_addsub(regs, n, AWP_OP_SUB);
				break;
			case AWP_MF:
				res = awp_float_mul(regs, n);
				break;
			case AWP_DF:
				res = awp_float_div(regs, n);
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
