//  Copyright (c) 2012-2013 Jakub Filipowicz <jakubf@gmail.com>
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

#include "cpu/cpu.h"
#include "cpu/reg/ir.h"
#include "mem/mem.h"
#include "cpu/iset.h"

#include "utils.h"

#include "dasm/dasm.h"
#include "dasm/dasm_iset.h"

static int dt_parse(struct dasm_opdef *opdef, uint16_t opcode, int nb, uint16_t addr, char *format, char *buf);
static int dt_opext(char *buf, uint16_t opcode);
static int dt_dasm_eff_arg(int nb, char *buf, uint16_t opcode, uint16_t addr);
static int dt_trans_eff_arg(int nb, char *buf, uint16_t opcode, uint16_t addr);

// -----------------------------------------------------------------------
int dt_trans(int nb, int addr, char *buf, int dasm_mode)
{
	struct dasm_opdef *opdef;
	int res;
	uint16_t opcode;

	res = mem_get(nb, addr, &opcode);

	if (!res) {
		sprintf(buf, "~~~~");
		return 1;
	}

	opdef = dasm_iset + _OP(opcode);
	if (opdef->e_opdef) {
		opdef = opdef->e_opdef + opdef->extop_fun(opcode);
	}

	char *format;
	switch (dasm_mode) {
		case DMODE_TRANS:
			format = opdef->t_format;
			break;
		case DMODE_DASM:
			format = opdef->d_format;
			break;
		default:
			format = NULL;
			break;
	}

	return dt_parse(opdef, opcode, nb, addr, format, buf);
}

// -----------------------------------------------------------------------
static int dt_parse(struct dasm_opdef *opdef, uint16_t opcode, int nb, uint16_t addr, char *format, char *buf)
{
	char *in = format;
	char *out = buf;
	char *b;
	int len = 1;

	if ((opdef->twoword) && (!_C(opcode))) {
		len++;
	}

	while ((in) && (*in)) {
		if (*in != '%') {
			*(out++) = *(in++);
		} else {
			switch (*(++in)) {
				case 'I':
					out += sprintf(out, "%-3s", opdef->mnemo);
					break;
				case 'E':
					/* out += dt_opext(out, opcode); */
					break;
				case 'A':
					out += sprintf(out, "%i", _A(opcode));
					break;
				case 'B':
					out += sprintf(out, "%i", _B(opcode));
					break;
				case 'C':
					out += sprintf(out, "%i", _C(opcode));
					break;
				case 'T':
					out += sprintf(out, "%i", _T(opcode));
					break;
				case 'R':
					out += sprintf(out, "0x%04x", addr + _T(opcode) + 1);
					break;
				case 't':
					out += sprintf(out, "%i", _t(opcode));
					break;
				case 'b':
					out += sprintf(out, "%i", _b(opcode));
					break;
				case 'N':
					out += dt_dasm_eff_arg(nb, out, opcode, addr+1);
					break;
				case 'n':
					out += dt_trans_eff_arg(nb, out, opcode, addr+1);
					break;
				case '0':
					b = int2binf("................", opcode, 16);
					out += sprintf(out, "%s", b);
					free(b);
					break;
				case 'x':
					out += sprintf(out, "0x%04x", opcode);
					break;
				case 'd':
					out += sprintf(out, "%i", opcode);
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

	*out = '\0';

	return len;
}

// -----------------------------------------------------------------------
static int dt_opext(char *buf, uint16_t opcode)
{
	int n = 0;

	if (_D(opcode) == 1) {
		if (_C(opcode) != 0) {
			n += sprintf(buf+n, "A");
		} else {
			n += sprintf(buf+n, "I");
		}
	} else {
		if (_C(opcode) != 0) {
			n += sprintf(buf+n, "R");
		} else {
			n += sprintf(buf+n, "D");
		}
	}

	return n;
}

// -----------------------------------------------------------------------
static int dt_dasm_eff_arg(int nb, char *buf, uint16_t opcode, uint16_t addr)
{
	int n = 0;
	int res;
	uint16_t arg;

	if (_D(opcode) == 1) {
		n += sprintf(buf+n, "[");
	}

	if (_C(opcode) != 0) {
		n += sprintf(buf+n, "r%i", _C(opcode));
	} else {
		res = mem_get(nb, addr, &arg);
		if (res) {
			n += sprintf(buf+n, "0x%x", arg);
		} else {
			n += sprintf(buf+n, "~~~~");
		}
	}

	if (_B(opcode) != 0) {
		n += sprintf(buf+n, "+r%i", _B(opcode));
	}

	if (_D(opcode) == 1) {
		n += sprintf(buf+n, "]");
	}

	return n;
}

// -----------------------------------------------------------------------
static int dt_trans_eff_arg(int nb, char *buf, uint16_t opcode, uint16_t addr)
{
	int n = 0;
	int res;
	uint16_t arg;

	if (_D(opcode) == 1) {
		n += sprintf(buf+n, "[");
	}

	if (_C(opcode) == 0) {
		res = mem_get(nb, addr, &arg);
		if (res) {
			n += sprintf(buf+n, "%i", arg);
		} else {
			n += sprintf(buf+n, "~~~~");
		}
	} else {
		n += sprintf(buf+n, "r%i", _C(opcode));
	}

	if (_B(opcode) != 0) {
		n += sprintf(buf+n, "+r%i", _B(opcode));
	}

	if (_D(opcode) == 1) {
		n += sprintf(buf+n, "]");
	}

	return n;
}

// vim: tabstop=4 shiftwidth=4 autoindent
