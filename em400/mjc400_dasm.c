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
#include <stdlib.h>
#include <string.h>
#include "mjc400_dasm.h"
#include "mjc400_iset.h"
#include "mjc400_mem.h"
#include "mjc400_regs.h"
#include "mjc400.h"
#include "utils.h"

// -----------------------------------------------------------------------
int mjc400_dasm(uint16_t* memptr, char **buf)
{
	uint8_t op = _OP(*memptr);
	return mjc400_iset[op].dasm_fun(op, memptr, buf);
}

// -----------------------------------------------------------------------
int mjc400_dasm_illegal(uint8_t op, uint16_t* memptr, char **buf)
{
	*buf = malloc(1024);
	char *opcode = int2bin(*memptr, 16);
	sprintf(*buf, "ILLEGAL: 0x%04x (dec: %i, bin: %s)", *memptr, *memptr, opcode);
	free(opcode);

	return 1;
}

// -----------------------------------------------------------------------
int mjc400_dasm_opext(char *buf, uint16_t *memptr)
{
	int n = 0;

	if (_D(*memptr) == 1) {
		if (_C(*memptr) != 0) {
			n += sprintf(buf+n, "A");
		} else {
			n += sprintf(buf+n, "I");
		}
	} else {
		if (_C(*memptr) != 0) {
			n += sprintf(buf+n, "R");
		} else {
			n += sprintf(buf+n, "D");
		}
	}

	return n;
}

// -----------------------------------------------------------------------
int mjc400_dasm_eff_arg(char *buf, uint16_t *memptr)
{
	int n = 0;

	if (_C(*memptr) != 0) {
		n += sprintf(buf+n, "%i", _C(*memptr));
	} else {
		n += sprintf(buf+n, "%i", *(memptr+1));
	}

	if (_B(*memptr) != 0) {
		n += sprintf(buf+n, ", r%i", _B(*memptr));
	}

	return n;
}

// -----------------------------------------------------------------------
int mjc400_dasm_2argn(uint8_t op, uint16_t* memptr, char **buf)
{
	*buf = malloc(1024);

	int n = 0;
	n += sprintf((*buf)+n, "%s", mjc400_iset[op].mnemo_assm);
	n += mjc400_dasm_opext((*buf)+n, memptr);
	n += sprintf((*buf)+n, " r%i, ", _A(*memptr));
	n += mjc400_dasm_eff_arg((*buf)+n, memptr);

	if (_C(*memptr) == 0) return 2;
	else return 1;

}

// -----------------------------------------------------------------------
int mjc400_dasm_37(uint8_t op, uint16_t* memptr, char **buf)
{
	return 0;
}

// -----------------------------------------------------------------------
int mjc400_dasm_fd(uint8_t op, uint16_t* memptr, char **buf)
{
	return 0;
}

// -----------------------------------------------------------------------
int mjc400_dasm_ka1(uint8_t op, uint16_t* memptr, char **buf)
{
	return 0;
}

// -----------------------------------------------------------------------
int mjc400_dasm_70(uint8_t op, uint16_t* memptr, char **buf)
{
	return 0;
}

// -----------------------------------------------------------------------
int mjc400_dasm_js(uint8_t op, uint16_t* memptr, char **buf)
{
	return 0;
}

// -----------------------------------------------------------------------
int mjc400_dasm_71(uint8_t op, uint16_t* memptr, char **buf)
{
	return 0;
}

// -----------------------------------------------------------------------
int mjc400_dasm_ka2(uint8_t op, uint16_t* memptr, char **buf)
{
	return 0;
}

// -----------------------------------------------------------------------
int mjc400_dasm_72(uint8_t op, uint16_t* memptr, char **buf)
{
	return 0;
}

// -----------------------------------------------------------------------
int mjc400_dasm_c(uint8_t op, uint16_t* memptr, char **buf)
{
	return 0;
}

// -----------------------------------------------------------------------
int mjc400_dasm_c_shc(uint8_t op, uint16_t* memptr, char **buf)
{
	return 0;
}

// -----------------------------------------------------------------------
int mjc400_dasm_73(uint8_t op, uint16_t* memptr, char **buf)
{
	return 0;
}

// -----------------------------------------------------------------------
int mjc400_dasm_s(uint8_t op, uint16_t* memptr, char **buf)
{
	return 0;
}

// -----------------------------------------------------------------------
int mjc400_dasm_74(uint8_t op, uint16_t* memptr, char **buf)
{
	return 0;
}

// -----------------------------------------------------------------------
int mjc400_dasm_j(uint8_t op, uint16_t* memptr, char **buf)
{
	return 0;
}

// -----------------------------------------------------------------------
int mjc400_dasm_75(uint8_t op, uint16_t* memptr, char **buf)
{
	return 0;
}

// -----------------------------------------------------------------------
int mjc400_dasm_l(uint8_t op, uint16_t* memptr, char **buf)
{
	return 0;
}

// -----------------------------------------------------------------------
int mjc400_dasm_76(uint8_t op, uint16_t* memptr, char **buf)
{
	return 0;
}

// -----------------------------------------------------------------------
int mjc400_dasm_g(uint8_t op, uint16_t* memptr, char **buf)
{
	return 0;
}

// -----------------------------------------------------------------------
int mjc400_dasm_77(uint8_t op, uint16_t* memptr, char **buf)
{
	return 0;
}

// -----------------------------------------------------------------------
int mjc400_dasm_bn(uint8_t op, uint16_t* memptr, char **buf)
{
	return 0;
}


