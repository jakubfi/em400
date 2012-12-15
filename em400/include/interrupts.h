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

extern uint32_t RZ;

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

#define INT_SET(x)		RZ |= x
#define INT_CLEAR(x)	RZ &= ~(x)

#endif

// vim: tabstop=4
