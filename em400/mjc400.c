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
#include "em400_debuger.h"
#include "mjc400.h"
#include "mjc400_mem.h"
#include "mjc400_regs.h"
#include "mjc400_iset.h"
#include "mjc400_instr.h"
#include "mjc400_timer.h"

// -----------------------------------------------------------------------
int mjc400_init()
{
	int res;
	mjc400_reset();
	res = mjc400_timer_start();
	return res;
}

// -----------------------------------------------------------------------
void mjc400_reset()
{
	IC = 0;
	IR = 0;
	SR = 0;
	for (int i=0 ; i<16 ; i++) {
		R[i] = 0;
	}
	RZ = 0;
	P = 0;
	MOD = 0;
	MODcnt = 0;
	KB = 0;

	mjc400_clear_mem();
}

// -----------------------------------------------------------------------
void mjc400_clear_mem()
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
int mjc400_load_os_image(const char* fname, uint16_t addr)
{
	if (addr > OS_MEM_BANK_SIZE) {
		return 1;
	}
	return __mjc400_load_image(fname, mjc400_os_mem+addr, OS_MEM_BANK_SIZE);
}
// -----------------------------------------------------------------------
int mjc400_load_user_image(const char* fname, unsigned short bank, uint16_t addr)
{
	if (bank > USER_MEM_BANK_COUNT) {
		return 1;
	}
	if (addr > USER_MEM_BANK_SIZE) {
		return 1;
	}
	return __mjc400_load_image(fname, mjc400_user_mem[bank]+addr, USER_MEM_BANK_SIZE);
}

// -----------------------------------------------------------------------
int __mjc400_load_image(const char* fname, uint16_t *ptr, uint16_t len)
{
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

// -----------------------------------------------------------------------
uint16_t mjc400_fetch_instr()
{
	IC += 1;
	if (SR_Q == 0) {
		return mjc400_os_mem[IC-1];
	} else {
		return mjc400_user_mem[SR_NB][IC-1];
	}
}

// -----------------------------------------------------------------------
int16_t mjc400_fetch_data()
{
	IC += 1;
	return MEM(IC-1);
}

// -----------------------------------------------------------------------
int16_t mjc400_get_eff_arg()
{
	int16_t N = 0;

	// argument is in next word
	if (IR_C == 0) {
		N += mjc400_fetch_data();
	// argument is in field C
	} else {
		N += R[IR_C];
	}

	// add B (if >0, adding 0 won't hurt)
	N += R[IR_B];

	// pre-modification
	N += MOD;
	
	// if D is set, N is an address in current memory block
	if (IR_D == 1) {
		N = MEM(N);
	}
	return N;
}

// -----------------------------------------------------------------------
void mjc400_step()
{
	// do not branch by default
	P = 0;

	// fetch instruction into IR
	// (additional argument is fetched by the instruction, if necessary)
	IR = mjc400_fetch_instr();

	// execute instruction
	int op_res;
	op_res = mjc400_iset[IR_OP].op_fun();

	switch (op_res) {
		// normal instruction
		case OP_OK:
			MOD = 0;
			MODcnt = 0;
			break;
		// pre-modification
		case OP_MD:
			break;
		// illegal instruction
		case OP_ILLEGAL:
			MOD = MODcnt = 0;
			if (P!=0) {
				P = 0;
			} else {
				RZ_6sb;
			}
			break;
	}

	IC += P;
}
