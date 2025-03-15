//  Copyright (c) 2012-2024 Jakub Filipowicz <jakubf@gmail.com>
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
#include <stdatomic.h>

#define INT_VECTORS 0x40
#define EXL_VECTOR 0x60
#define STACK_POINTER 0x61

extern uint32_t rz;
extern atomic_bool irq;

enum named_interrupts {
	INT_2CPU_POWER		= 0,
	INT_PARITY			= 1,
	INT_NO_MEM			= 2,
	INT_2CPU_HIGH		= 3,
	INT_IFACE_POWER		= 4,
	INT_CLOCK			= 5,
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

enum int_masks {
	MASK_0  = 0b0000000000,
	MASK_1  = 0b1000000000,
	MASK_2  = 0b1100000000,
	MASK_3  = 0b1110000000,
	MASK_4  = 0b1111000000,
	MASK_5  = 0b1111100000,
	MASK_6  = 0b1111110000,
	MASK_7  = 0b1111111000,
	MASK_8  = 0b1111111100,
	MASK_9  = 0b1111111110,
	MASK_EX = MASK_4,
};

void int_update_xmask();
void int_set(int int_num);
void int_clear(int int_num);
void int_clear_all();
void int_put_nchan(uint16_t r);
uint16_t int_get_nchan();
uint16_t int_get_chan();
void int_serve();
void int_ctx_switch(uint16_t int_spec, uint16_t new_ic, uint16_t new_rm);
void int_ctx_restore(bool barnb);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
