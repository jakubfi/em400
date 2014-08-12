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

#ifndef DEBUGGER_DASM_H
#define DEBUGGER_DASM_H

#include <inttypes.h>

#include "debugger/dasm_iset.h"

#define DMODE_DASM	1
#define DMODE_TRANS	2

// macros to access sub-opcodes
#define EXT_OP_37(x) _A(x)
#define EXT_OP_70(x) _A(x)
#define EXT_OP_71(x) ((x & 0b0000001100000000) >> 8)
#define EXT_OP_72(x) (((x & 0b0000001000000000) >> 3) | (x & 0b0000000000111111))
#define EXT_OP_73(x) (((x & 0b0000001111000000) >> 3) | (x & 0b0000000000000111))
#define EXT_OP_74(x) _A(x)
#define EXT_OP_75(x) _A(x)
#define EXT_OP_76(x) _A(x)
#define EXT_OP_77(x) _A(x)

int dt_trans(int addr, char *buf, int dasm_mode);
int dt_parse(struct dasm_opdef *opdef, uint16_t opcode, uint16_t addr, char *format, char *buf);
int dt_dasm_eff_arg(char *buf, uint16_t opcode, uint16_t addr);
int dt_trans_eff_arg(char *buf, uint16_t opcode, uint16_t addr);
int dt_opext(char *buf, uint16_t opcode);

int dt_extcode_37(int i);
int dt_extcode_70(int i);
int dt_extcode_71(int i);
int dt_extcode_72(int i);
int dt_extcode_73(int i);
int dt_extcode_74(int i);
int dt_extcode_75(int i);
int dt_extcode_76(int i);
int dt_extcode_77(int i);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
