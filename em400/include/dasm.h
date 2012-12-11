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

#ifndef DASM_H
#define DASM_H

#include <inttypes.h>

#include "dasm_iset.h"

#define DMODE_DASM	1
#define DMODE_TRANS	2

int dt_trans(uint16_t* memptr, char **buf, int dasm_mode);
int dt_parse(struct dasm_opdef *opdef, uint16_t *memptr, char *format, char *buf);
int dt_dasm_eff_arg(char *buf, uint16_t *memptr);
int dt_trans_eff_arg(char *buf, uint16_t *memptr);
int dt_opext(char *buf, uint16_t *memptr);

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

// vim: tabstop=4
