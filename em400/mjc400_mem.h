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

#ifndef MJC400_MEM_H
#define MJC400_MEM_H

#include <inttypes.h>

#define OS_MEM_BANK_SIZE 8 * 1024
#define USER_MEM_BANK_SIZE 32 * 1024
#define USER_MEM_BANK_COUNT 16

// -----------------------------------------------------------------------
// memory
extern uint16_t mjc400_os_mem[OS_MEM_BANK_SIZE];
extern uint16_t mjc400_user_mem[USER_MEM_BANK_COUNT][USER_MEM_BANK_SIZE];

// -----------------------------------------------------------------------
// unified OS/user memory access macro
#define MEM(a) SR_Q ? mjc400_user_mem[SR_NB][a] : mjc400_os_mem[a]
#define MEMNB(a) SR_NB ? mjc400_user_mem[SR_NB][a] : mjc400_os_mem[a]
#define MEMw(a, x) \
	if (SR_Q == 1) mjc400_user_mem[SR_NB][a] = x; \
	else mjc400_os_mem[a] = x;
#define MEMNBw(a, x) \
	if (SR_NB == 1) mjc400_user_mem[SR_NB][a] = x; \
	else mjc400_os_mem[a] = x;

#define MEMptr(a) SR_Q ? mjc400_user_mem[SR_NB]+a : mjc400_os_mem+a
#define MEMNBptr(a) SR_NB ? mjc400_user_mem[SR_NB]+a : mjc400_os_mem+a

// -----------------------------------------------------------------------
// dword conversions
#define DWORD(x, y) (x<<16) | (y)
#define DWORDw(x, y, z) x = (z>>16) & 0xffff; y = (z) & 0xffff;

#endif

// vim: tabstop=4
