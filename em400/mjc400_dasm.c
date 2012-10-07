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
	struct mjc400_opdef *opdef;
	*buf = malloc(1024);

	opdef = mjc400_iset + _OP(*memptr);
	if (!(opdef->d_format)) {
		opdef = opdef->e_opdef + opdef->extop_fun(*memptr);
	}

	return mjc400_dasm_parse(opdef, memptr, opdef->d_format, *buf);
}

// -----------------------------------------------------------------------
int mjc400_dasm_parse(struct mjc400_opdef *opdef, uint16_t *memptr, char *format, char *buf)
{
	char *in = format;
	char *out = buf;
	char *b;
	int len = 1;

	if ((opdef->twoword) && (!_C(*memptr))) {
		len++;
	}

	while (*in) {
		if (*in != '%') {
			*(out++) = *(in++);
		} else {
			switch (*(++in)) {
				case 'I':
					out += sprintf(out, "%s", opdef->mnemo);
					break;
				case 'E':
					out += mjc400_dasm_opext(out, memptr);
					break;
				case 'A':
					out += sprintf(out, "%i", _A(*memptr));
					break;
				case 'B':
					out += sprintf(out, "%i", _B(*memptr));
					break;
				case 'C':
					out += sprintf(out, "%i", _C(*memptr));
					break;
				case 'T':
					out += sprintf(out, "%i", _T(*memptr));
					break;
				case 't':
					out += sprintf(out, "%i", _t(*memptr));
					break;
				case 'b':
					out += sprintf(out, "%i", _b(*memptr));
					break;
				case 'N':
					out += mjc400_dasm_eff_arg(out, memptr);
					break;
				case '0':
					b = int2bin(*memptr, 16);
					out += sprintf(out, "%s", b);
					free(b);
					break;
				case 'x':
					out += sprintf(out, "0x%04x", *memptr);
					break;
				case 'd':
					out += sprintf(out, "%i", *memptr);
					break;
				case '%':
					*(out++) = '%';
					break;
				default:
					*(out++) = '%';
					*(out++) = *in;
			}
			in++;
		}
	}
	return len;
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
		n += sprintf(buf+n, "r%i", _C(*memptr));
	} else {
		n += sprintf(buf+n, "%i", *(memptr+1));
	}

	if (_B(*memptr) != 0) {
		n += sprintf(buf+n, ", r%i", _B(*memptr));
	}

	return n;
}

// -----------------------------------------------------------------------
int mjc400_dasm_e37(int i)
{
	return EXT_OP_37(i);
}

// -----------------------------------------------------------------------
int mjc400_dasm_e70(int i)
{
	return EXT_OP_70(i);
}

// -----------------------------------------------------------------------
int mjc400_dasm_e71(int i)
{
	return EXT_OP_71(i);
}

// -----------------------------------------------------------------------
int mjc400_dasm_e72(int i)
{
	return EXT_OP_72(i);
}

// -----------------------------------------------------------------------
int mjc400_dasm_e73(int i)
{
	return EXT_OP_73(i);
}

// -----------------------------------------------------------------------
int mjc400_dasm_e74(int i)
{
	return EXT_OP_74(i);
}

// -----------------------------------------------------------------------
int mjc400_dasm_e75(int i)
{
	return EXT_OP_75(i);
}

// -----------------------------------------------------------------------
int mjc400_dasm_e76(int i)
{
	return EXT_OP_76(i);
}

// -----------------------------------------------------------------------
int mjc400_dasm_e77(int i)
{
	return EXT_OP_77(i);
}

// vim: tabstop=4
