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

#ifndef MJC400_DASM_H
#define MJC400_DASM_H

#include <inttypes.h>
#include "mjc400_iset.h"

/*
	Disassembler output formatting:

	%I - mnemonic
	%E - mnemonic suffix
	%A - reg. A
	%B - reg. B
	%C - reg. C
	%T - short arg.
	%t - very short arg. (SHC only)
	%b - byte arg.
	%N - norm. arg. (disassembler syntax)
	%0 - bin representation of a word
	%x - hex representation of a word
	%d - dec representation of a word
*/

#define DT_ILL	"ILLEGAL: %0 (0x%x)"
#define D_2ARGN	"%I%E r%A, %N"
#define D_1ARGN	"%I%E %N"
#define D_FD	D_1ARGN
#define D_KA1	"%I r%A, %T"
#define D_JS	"%I %T"
#define D_KA2	"%I %b"
#define D_C		"%I r%A"
#define D_SHC	"%I r%A, %t"
#define D_S		"%I"
#define D_J		D_1ARGN
#define D_L		D_1ARGN
#define D_G		D_1ARGN
#define D_BN	D_1ARGN

int mjc400_dasm_e37(int i);
int mjc400_dasm_e70(int i);
int mjc400_dasm_e71(int i);
int mjc400_dasm_e72(int i);
int mjc400_dasm_e73(int i);
int mjc400_dasm_e74(int i);
int mjc400_dasm_e75(int i);
int mjc400_dasm_e76(int i);
int mjc400_dasm_e77(int i);

int mjc400_dasm(uint16_t* memptr, char **buf);
int mjc400_dasm_parse(struct mjc400_opdef *opdef, uint16_t *memptr, char *format, char *buf);
int mjc400_dasm_eff_arg(char *buf, uint16_t *memptr);
int mjc400_dasm_opext(char *buf, uint16_t *memptr);

#endif

// vim: tabstop=4
