//  Copyright (c) 2012 Jakub Filipowicz <jakubf@gmail.com>
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

#ifdef WITH_DEBUGGER
#include "debugger_ui.h"
#endif
#include "registers.h"

uint16_t regs[R_MAX];
#ifdef WITH_DEBUGGER
int reg_act[R_MAX];
#endif

uint32_t RZ;

// -----------------------------------------------------------------------
uint16_t reg_read(unsigned short int r)
{
	return regs[r];
}

// -----------------------------------------------------------------------
void reg_write(unsigned short int r, uint16_t x)
{
#ifdef WITH_DEBUGGER
	reg_act[r] = C_WRITE;
#endif
	if (r) {
		regs[r] = x;
	} else {
		regs[r] = regs[r] | (x & 0b0000000011111111);
	}
}

// vim: tabstop=4
