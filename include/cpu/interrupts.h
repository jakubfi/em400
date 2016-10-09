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

#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <inttypes.h>
#include <pthread.h>

extern uint32_t RZ;
extern uint32_t RP;

enum _interrupts {
	INT_2CPU_POWER		= 0,
	INT_PARITY			= 1,
	INT_NO_MEM			= 2,
	INT_2CPU_HIGH		= 3,
	INT_IFACE_POWER		= 4,
	INT_TIMER			= 5,
	INT_ILLEGAL_INSTRUCTION	= 6,
	INT_DIV_OF			= 7,
	INT_FP_UF			= 8,
	INT_FP_OF			= 9,
	INT_FP_ERR			= 10,
	INT_EXTRA			= 11,
	INT_C0				= 12,
	INT_C1				= 13,
	INT_C2				= 14,
	INT_C3				= 15,
	INT_C4				= 16,
	INT_C5				= 17,
	INT_C6				= 18,
	INT_C7				= 19,
	INT_C8				= 20,
	INT_C9				= 21,
	INT_C10				= 22,
	INT_C11				= 23,
	INT_C12				= 24,
	INT_C13				= 25,
	INT_C14				= 26,
	INT_C15				= 27,
	INT_OPRQ			= 28,
	INT_2CPU_LOW		= 29,
	INT_SOFT_U			= 30,
	INT_SOFT_L			= 31
};
enum int_masks_e {
	MASK_0  = 0b0000000000111111,
	MASK_1  = 0b1000000000111111,
	MASK_2  = 0b1100000000111111,
	MASK_3  = 0b1110000000111111,
	MASK_4  = 0b1111000000111111,
	MASK_5  = 0b1111100000111111,
	MASK_6  = 0b1111110000111111,
	MASK_7  = 0b1111111000111111,
	MASK_8  = 0b1111111100111111,
	MASK_9  = 0b1111111110111111,
	MASK_Q  = 0b1111111111011111,
	MASK_EX = MASK_4,
};

void int_update_mask(uint16_t mask);
void int_set(int x);
void int_clear(int x);
void int_clear_all();
void int_put_nchan(uint16_t r);
uint16_t int_get_nchan();
void int_serve();

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
