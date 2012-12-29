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

#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <inttypes.h>
#include <pthread.h>

extern pthread_cond_t int_cond;
extern pthread_mutex_t int_mutex;

extern volatile uint32_t RZ;
extern volatile uint32_t RP;
extern uint32_t xmask;

extern const int int_int2rm[32];
extern const int int_int2mask[32];

#define INT_2CPU_POWER		1 << (31 - 0)
#define INT_PARITY			1 << (31 - 1)
#define INT_NO_MEM			1 << (31 - 2)
#define INT_2CPU_HIGH		1 << (31 - 3)
#define INT_IFACE_POWER		1 << (31 - 4)
#define INT_TIMER			1 << (31 - 5)
#define INT_ILLEGAL_OPCODE	1 << (31 - 6)
#define INT_FP_DIV_OF		1 << (31 - 7)
#define INT_FP_OF			1 << (31 - 8)
#define INT_FP_UF			1 << (31 - 9)
#define INT_DIV0			1 << (31 - 10)
#define INT_EXTRA			1 << (31 - 11)
#define INT_C0				1 << (31 - 12)
#define INT_C1				1 << (31 - 13)
#define INT_C2				1 << (31 - 14)
#define INT_C3				1 << (31 - 15)
#define INT_C4				1 << (31 - 16)
#define INT_C5				1 << (31 - 17)
#define INT_C6				1 << (31 - 18)
#define INT_C7				1 << (31 - 19)
#define INT_C8				1 << (31 - 20)
#define INT_C9				1 << (31 - 21)
#define INT_C10				1 << (31 - 22)
#define INT_C11				1 << (31 - 23)
#define INT_C12				1 << (31 - 24)
#define INT_C13				1 << (31 - 25)
#define INT_C14				1 << (31 - 26)
#define INT_C15				1 << (31 - 27)
#define INT_OPRQ			1 << (31 - 28)
#define INT_2CPU_LOW		1 << (31 - 29)
#define INT_SOFT_U			1 << (31 - 30)
#define INT_SOFT_L			1 << (31 - 31)

#define INT_ALL				0xffffffff

#define MASK_0				0b0000000000111111
#define MASK_1				0b1000000000111111
#define MASK_2				0b1100000000111111
#define MASK_3				0b1110000000111111
#define MASK_4				0b1111000000111111
#define MASK_5				0b1111100000111111
#define MASK_6				0b1111110000111111
#define MASK_7				0b1111111000111111
#define MASK_8				0b1111111100111111
#define MASK_9				0b1111111110111111

void int_update_rp();
void int_set(uint32_t x);
void int_clear(uint32_t x);
void int_put_nchan(uint16_t r);
uint16_t int_get_nchan();
void int_mask_below(int i);

void int_serve();

#endif

// vim: tabstop=4
