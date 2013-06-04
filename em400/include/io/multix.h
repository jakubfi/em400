//  Copyright (c) 2013 Jakub Filipowicz <jakubf@gmail.com>
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

#ifndef DRV_MULTIX_H
#define DRV_MULTIX_H

#include <inttypes.h>

#include "cfg.h"
#include "io.h"

enum mx_int_e {
	MX_INT_INSKA = 1,
	MX_INT_IWYZE = 2,
	MX_INT_IWYTE = 3,
	MX_INT_INKON = 4,
	MX_INT_IUKON = 5,
	MX_INT_INKOT = 6,
	MX_INT_ISTRE = 7,
	MX_INT_INSTR = 8,
	MX_INT_INKST = 9,
	MX_INT_IDOLI = 10,
	MX_INT_INDOL = 11,
	MX_INT_INKDO = 12,
	MX_INT_IETRA = 13,
	MX_INT_INTRA = 14,
	MX_INT_INKTR = 15,
	MX_INT_ITRER = 16,
	MX_INT_ITRAB = 19,
	MX_INT_IABTR = 20,
	MX_INT_INABT = 21,
	MX_INT_INKAB = 22,
	MX_INT_IODLI = 23,
	MX_INT_INODL = 24,
	MX_INT_INKOD = 25,
	MX_INT_INPAO = 32,
	MX_INT_IPARE = 33,
	MX_INT_IOPRU = 34
};

int drv_multix_init(void *self, struct cfg_arg_t *arg);
void drv_multix_shutdown(void *self);
void drv_multix_reset(void *self);
int drv_multix_cmd(void *self, int dir, uint16_t n_arg, uint16_t *r_arg);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
