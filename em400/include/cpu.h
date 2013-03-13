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

#ifndef CPU_H
#define CPU_H

#include <inttypes.h>

void cpu_reset();
void cpu_step();
int16_t get_arg_short();
#ifdef WITH_DEBUGGER
int16_t get_arg_norm();
#else
extern uint32_t __N;
#define get_arg_norm() (int16_t) \
( __N = \
	IR_D ? \
		MEM( \
		(IR_C ? R(IR_C) : (nMEM(nR(R_IC))) \
		+ (IR_B ? R(IR_B) : 0) \
		+ nR(R_MOD)) \
		) \
	: \
		(IR_C ? R(IR_C) : nMEM(nR(R_IC))) \
		+ (IR_B ? R(IR_B) : 0) \
		+ nR(R_MOD) \
); \
if (!IR_C) nRinc(R_IC); \
if (em400_cfg.cpu.mod_17bit) nRw(R_ZC17, (__N >> 16) & 1);
#endif

#endif

// vim: tabstop=4
