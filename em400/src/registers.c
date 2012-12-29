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

#include "registers.h"
#include "interrupts.h"

#ifdef WITH_DEBUGGER
#include "debugger/debugger.h"
#include "debugger/ui.h"
#endif

uint16_t regs[R_MAX];

// -----------------------------------------------------------------------
uint16_t reg_read(int r, int trace)
{
#ifdef WITH_DEBUGGER
	if (trace != 0) {
		if (reg_act[r] == C_WRITE) {
			reg_act[r] = C_RW;
		} else {
			reg_act[r] = C_READ;
		}
	}
#endif
	return regs[r];
}

// -----------------------------------------------------------------------
void reg_write(int r, uint16_t x, int trace)
{
#ifdef WITH_DEBUGGER
	if (trace != 0) {
		if (reg_act[r] == C_READ) {
			reg_act[r] = C_RW;
		} else {
			reg_act[r] = C_WRITE;
		}
	}
#endif
	if (r) {
		// if this is not SR, just do it
		if (r != R_SR) {
			regs[r] = x;
		// if this is SR, user may have changed RM
		} else {
			// if RM is untouched, just do it
			if ((regs[R_SR] & 0b1111111111000000) == (r & 0b1111111111000000)) {
				regs[r] = x;
			// if RM has changed, we need to update RP
			} else {
				pthread_mutex_lock(&int_mutex);
				regs[r] = x;
				int_update_rp();
				pthread_mutex_unlock(&int_mutex);
			}
		}
	} else {
		regs[r] = regs[r] | (x & 0b0000000011111111);
	}
}

// vim: tabstop=4
