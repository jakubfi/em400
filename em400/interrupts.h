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

#include "registers.h"

#define INT_2CPU_POWER		0
#define INT_PARITY			1
#define INT_NO_MEM			2
#define INT_2CPU_HIGH		3
#define INT_IFACE_POWER		4
#define INT_TIMER			5
#define INT_ILLEGAL_OPCODE	6
#define INT_FP_DIV_OF		7
#define INT_FP_OF			8
#define INT_FP_UF			9
#define INT_DIV0			10
#define INT_EXTRA			11
#define INT_C0				12
#define INT_C1				13
#define INT_C2				14
#define INT_C3				15	
#define INT_C4				16
#define INT_C5				17
#define INT_C6				18
#define INT_C7				19
#define INT_C8				20
#define INT_C9				21
#define INT_C10				22
#define INT_C11				23
#define INT_C12				24
#define INT_C13				25
#define INT_C14				26
#define INT_C15				27
#define INT_OPRQ			28
#define INT_2CPU_LOW		29
#define INT_SOFT_U			30
#define INT_SOFT_L			31

#define INT_SET(x)		RZ |= ((uint32_t)1 << x)
#define INT_CLEAR(x)	RZ &= ~((uint32_t)1 << x)

#endif

// vim: tabstop=4
