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

#include "registers.h"

#ifdef WITH_DEBUGGER
#include "debugger/debugger.h"
#include "debugger/ui.h"
#endif
#include "debugger/log.h"

uint16_t regs[R_MAX];

// -----------------------------------------------------------------------
uint16_t reg_read(int r, int trace)
{
#ifdef WITH_DEBUGGER
	if (trace != 0) {
		LOG(D_REG, 10, "%s -> 0x%04x", log_reg_name[r], regs[r]);
		if (reg_act[r] == C_WRITE) {
			reg_act[r] = C_RW;
		} else {
			reg_act[r] = C_READ;
		}
	} else {
		LOG(D_REG, 100, "%s -> 0x%04x", log_reg_name[r], regs[r]);
	}
#endif
	return regs[r];
}

// -----------------------------------------------------------------------
void reg_write(int r, uint16_t x, int trace, int hw)
{
#ifdef WITH_DEBUGGER
	LOG(D_REG, 1, "%s <- 0x%04x", log_reg_name[r], x);
	if (trace != 0) {
		if (reg_act[r] == C_READ) {
			reg_act[r] = C_RW;
		} else {
			reg_act[r] = C_WRITE;
		}
	}
#endif
	if (r || hw) {
		regs[r] = x;
	} else {
		regs[r] = (regs[r] & 0b1111111100000000) | (x & 0b0000000011111111);
	}
}

// vim: tabstop=4
