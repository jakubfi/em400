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

#include <stdio.h>
#include <arpa/inet.h>
#include "em400_routines.h"
#include "mjc400.h"
#include "em400_mem.h"
#include "mjc400_regs.h"
#include "mjc400_timer.h"

// -----------------------------------------------------------------------
int em400_init()
{
	int res;
	mjc400_reset();
	em400_clear_mem();
	res = mjc400_timer_start();
	return res;
}

// -----------------------------------------------------------------------
void em400_clear_mem()
{
	unsigned short bank;
	uint16_t addr;

	for (addr=0 ; addr<OS_MEM_BANK_SIZE ; addr++) {
		mjc400_os_mem[addr] = 0;
	}

	for (bank=0 ; bank<USER_MEM_BANK_COUNT ; bank++) {
		for (addr=0 ; addr<USER_MEM_BANK_SIZE ; addr++) {
			mjc400_user_mem[bank][addr] = 0;
		}
	}
}

// -----------------------------------------------------------------------
int em400_load_image(const char* fname, unsigned short bank, uint16_t addr)
{
	int len;
	uint16_t *ptr;

	if (bank < 0) {
		return 1;
	} else if (bank == 0) {
		if (addr > OS_MEM_BANK_SIZE) {
			return 1;
		} else {
			len = OS_MEM_BANK_SIZE;
			ptr = mjc400_os_mem+addr;
		}
	} else if (bank <= USER_MEM_BANK_COUNT) {
		if (addr > USER_MEM_BANK_SIZE) {
			return 1;
		} else {
			len = USER_MEM_BANK_SIZE;
			ptr = mjc400_user_mem[bank]+addr;
		}
	} else {
		return 1;
	}

	FILE *f = fopen(fname, "rb");

	if (f == NULL) {
		return 1;
	}

	uint16_t *i = ptr;

	int res = fread((void*)ptr, sizeof(*ptr), len, f);

	if (ferror(f)) {
		fclose(f);
		return 1;
	}

	fclose(f);

	// we swap bytes from big-endian to host-endianness at load time
	while (i < ptr + res) {
		*i = ntohs(*i);
		i++;
	}

	return 0;
}

// vim: tabstop=4
