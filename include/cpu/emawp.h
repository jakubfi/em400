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

#ifndef AWP_H
#define AWP_H

#include <inttypes.h>

enum awp_addsub_op {
	OP_ADD = 1,
	OP_SUB = -1,
};

enum awp_errors {
	AWP_OK,
	AWP_FP_UF,
	AWP_FP_OF,
	AWP_CRITICAL, // result > AWP_CRITICAL indicates that CPU register should stay unchanged
	AWP_DIV_OF,
	AWP_FP_ERR,
};

struct awp {
	uint16_t r1, r2, r3;
	uint16_t flags;
	int v;
};

struct awp * awp_init();
void awp_destroy(struct awp *awp);

void awp_load(struct awp *awp, uint16_t flags, uint16_t r1, uint16_t r2, uint16_t r3);
void awp_store(struct awp *awp, uint16_t *flags, uint16_t *r1, uint16_t *r2, uint16_t *r3);

int awp_dword_add(struct awp *awp, uint16_t b1, uint16_t b2, int op);
int awp_dword_mul(struct awp *awp, int16_t b);
int awp_dword_div(struct awp *awp, int16_t b);

int awp_float_norm(struct awp *awp);
int awp_float_add(struct awp *awp, uint16_t d1, uint16_t d2, uint16_t d3, int sign);
int awp_float_mul(struct awp *awp, uint16_t d1, uint16_t d2, uint16_t d3);
int awp_float_div(struct awp *awp, uint16_t d1, uint16_t d2, uint16_t d3);

int awp_to_double(double *f, uint16_t d1, uint16_t d2, uint16_t d3);
int awp_from_double(uint16_t *d1, uint16_t *d2, uint16_t *d3, uint16_t *flags, double f);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
