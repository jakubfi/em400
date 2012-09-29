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
int mjc400_dasm_2argn(uint8_t op, uint16_t* memptr, char **buf)
{
	int n = 0;
	*buf = malloc(1024);
	n += sprintf(*buf, "%s ", mjc400_iset[op].mnemo_assm);
	return 2;
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


