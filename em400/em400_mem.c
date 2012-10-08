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

#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include "em400_mem.h"
#include "mjc400_regs.h"

// memory configuration: number of 4Kword chunks in a bank
int em400_mem_bank_layout[MEM_BANKS] = { 2, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8 };
int em400_mem_bank_size[MEM_BANKS] = { -1 };

uint16_t *em400_mem[MEM_BANKS] = { NULL };

// -----------------------------------------------------------------------
int em400_mem_init()
{
	for (int i=0 ; i<MEM_BANKS ; i++) {
		// i <= 0 - bank not populated
		if (em400_mem_bank_layout[i] <= 0) {
			em400_mem[i] = NULL;
			em400_mem_bank_size[i] = -1;
		// 8 >= i > 0 - bank properly populated with n chunks of memory
		} else if (em400_mem_bank_layout[i] <= 8) {
			em400_mem[i] = malloc(em400_mem_bank_layout[i] * MEM_CHUNK * 2);
			if (em400_mem[i]) {
				em400_mem_bank_size[i] = em400_mem_bank_layout[i] * MEM_CHUNK;
			} else {
				em400_mem_bank_size[i] = -1;
				return 1;
			}
		// i > 8 - impossible memory bank configuration
		} else {
			em400_mem_bank_size[i] = -1;
			return 1;
		}
	}
	return 0;
}

// -----------------------------------------------------------------------
int em400_mem_shutdown()
{
	for (int i=0 ; i<MEM_BANKS ; i++) {
		free(em400_mem[i]);
	}
	return 0;
}

// -----------------------------------------------------------------------
// read from any bank
uint16_t em400_mem_read(short unsigned int bank, uint16_t addr)
{
	if (addr < em400_mem_bank_size[bank]) {
		return em400_mem[bank][addr];
	} else {
		if (SR_Q) {
			RZ_2sb;
		} else {
			// TODO: ALARM
		}
		return 0;
	}
}

// -----------------------------------------------------------------------
// write to any bank
void em400_mem_write(short unsigned int bank, uint16_t addr, uint16_t val)
{
	if (addr < em400_mem_bank_size[bank]) {
		em400_mem[bank][addr] = val;
	} else {
		if (SR_Q) {
			RZ_2sb;
		} else {
			// TODO: ALARM
		}
	}
}

// -----------------------------------------------------------------------
uint16_t * em400_mem_ptr(short unsigned int bank, uint16_t addr)
{
	if ((bank >= 0) && (bank < MEM_BANKS) && (addr < em400_mem_bank_size[bank])) {
		return em400_mem[bank] + addr;
	} else {
		return NULL;
	}
}

// -----------------------------------------------------------------------
void em400_mem_clear()
{
	unsigned short bank;
	uint16_t addr;

	for (bank=0 ; bank<MEM_BANKS ; bank++) {
		for (addr=0 ; addr<em400_mem_bank_size[bank] ; addr++) {
			em400_mem[bank][addr] = 0;
		}
	}
}

// -----------------------------------------------------------------------
int em400_mem_load_image(const char* fname, unsigned short bank, uint16_t addr)
{
	int len;
	uint16_t *ptr;

	if (bank < 0) {
		return 1;
	} else if (bank < MEM_BANKS) {
		if (addr > em400_mem_bank_size[bank]) {
			return 1;
		} else {
			len = em400_mem_bank_size[bank] - addr;
			ptr = em400_mem[bank] + addr;
		}
	} else {
		return 1;
	}

	FILE *f = fopen(fname, "rb");
	if (f == NULL) {
		return 1;
	}

	int res = fread((void*)ptr, sizeof(*ptr), len, f);
	if (ferror(f)) {
		fclose(f);
		return 1;
	}
	fclose(f);

	// we swap bytes from big-endian to host-endianness at load time
	uint16_t *i = ptr;
	while (i < ptr + res) {
		*i = ntohs(*i);
		i++;
	}

	return 0;
}


// vim: tabstop=4
