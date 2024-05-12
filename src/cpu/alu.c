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
		FSET(FL_E);
		FCLR(FL_G | FL_L);
	} else {
		if (a < b) {
			FSET(FL_L);
			FCLR(FL_E | FL_G);
		} else {
			FCLR(FL_E | FL_L);
			FSET(FL_G);
		}
	}
}

// -----------------------------------------------------------------------
void alu_16_set_Z_bool(uint16_t z)
{
	if (z == 0) FSET(FL_Z);
	else FCLR(FL_Z);
}

// -----------------------------------------------------------------------
void alu_16_add(int16_t reg, int16_t n, unsigned carry)
{
	int sres = reg + n + carry;
	unsigned ures = (uint16_t) reg + (uint16_t) n + carry;

	if (sres == 0) {
		FSET(FL_Z);
	} else {
		FCLR(FL_Z);
	}

	if (ures & BIT_MINUS_1) {
		FSET(FL_C);
	} else {
		FCLR(FL_C);
	}

	// Straight from the schematic:
	// Set M when one of the following is true:
	//  * C=1 and n[0] == r[0]
	//  * C=0 and n[0] != r[0]
	int diff_bit_zero = ((reg ^ n) & BIT_0) >> 15;
	if (FGET(FL_C) != diff_bit_zero) {
		FSET(FL_M);
	} else {
		FCLR(FL_M);
	}

	// NOTE: straight from the schematic
	if (((ures & BIT_0) >> 15) ^ FGET(FL_M)) {
		FSET(FL_V);
	} else {
		// V is never cleared
	}

	w = ures;
	REG_RESTRICT_WRITE(IR_A, w);
}

// -----------------------------------------------------------------------
void alu_16_sub(int16_t reg, int16_t n)
{
	int sres = reg - n;
	unsigned ures = (uint16_t) reg + (uint16_t) -n;

	if (sres == 0) {
		FSET(FL_Z);
	} else {
		FCLR(FL_Z);
	}

	// NOTE: x-0 always sets carry
	if ((ures & BIT_MINUS_1) || (n == 0)) {
		FSET(FL_C);
	} else {
		FCLR(FL_C);
	}

	// straight from the schematic
	// Set M when one of the following is true:
	//  * C=1 and n[0] != r[0]
	//  * C=0 and n[0] == r[0]
	int diff_bit_zero = ((reg ^ n) & BIT_0) >> 15;
	if (FGET(FL_C) == diff_bit_zero) {
		FSET(FL_M);
	} else {
		FCLR(FL_M);
	}

	// NOTE: straight from the schematic
	if (((ures & BIT_0) >> 15) ^ FGET(FL_M)) {
		FSET(FL_V);
	} else {
		// V is never cleared
	}
	REG_RESTRICT_WRITE(IR_A, ures);
}

// -----------------------------------------------------------------------
// ---- AWP --------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
void awp_dispatch(int op, uint16_t arg)
{
	assert(op >= AWP_NRF0);
	assert(op <= AWP_DF);

	static const int awp_arg_count[] = {
		0, // AWP_NRF0
		0, // AWP_NRF1
		0, // AWP_NRF2
		0, // AWP_NRF3
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
		uint16_t n[3];
		for (int i=0 ; i<awp_arg_count[op] ; i++) {
			if (!cpu_mem_read_1(q, arg+i, n+i)) return;
		}
		int res = 0;
		switch (op) {
			case AWP_NRF0:
			case AWP_NRF1:
			case AWP_NRF2:
			case AWP_NRF3:
				res = awp_float_norm(r);
				break;
			case AWP_AD:
				res = awp_dword_add(r, n);
				break;
			case AWP_SD:
				res = awp_dword_sub(r, n);
				break;
			case AWP_MW:
				res = awp_dword_mul(r, n[0]);
				break;
			case AWP_DW:
				res = awp_dword_div(r, n[0]);
				break;
			case AWP_AF:
				res = awp_float_add(r, n);
				break;
			case AWP_SF:
				res = awp_float_sub(r, n);
				break;
			case AWP_MF:
				res = awp_float_mul(r, n);
				break;
			case AWP_DF:
				res = awp_float_div(r, n);
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
		if (!cpu_mem_read_1(false, AWP_DISPATCH_TAB_ADDR+op, &ar)) return;
		cpu_ctx_switch(arg, ar, MASK_9);
	}
}

// vim: tabstop=4 shiftwidth=4 autoindent
