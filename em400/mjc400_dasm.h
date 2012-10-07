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

#define DMODE_DASM	1
#define DMODE_TRANS	2

int mjc400_dt(uint16_t* memptr, char **buf, int dasm_mode);
int mjc400_dt_parse(struct mjc400_opdef *opdef, uint16_t *memptr, char *format, char *buf);
int mjc400_dt_eff_arg(char *buf, uint16_t *memptr);
int mjc400_trans_eff_arg(char *buf, uint16_t *memptr);
int mjc400_dt_opext(char *buf, uint16_t *memptr);

int mjc400_dt_e37(int i);
int mjc400_dt_e70(int i);
int mjc400_dt_e71(int i);
int mjc400_dt_e72(int i);
int mjc400_dt_e73(int i);
int mjc400_dt_e74(int i);
int mjc400_dt_e75(int i);
int mjc400_dt_e76(int i);
int mjc400_dt_e77(int i);

#endif

// vim: tabstop=4
