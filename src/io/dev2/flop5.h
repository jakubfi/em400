//  Copyright (c) 2025 Jakub Filipowicz <jakubf@gmail.com>
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

#ifndef FLOP5_H
#define FLOP5_H

#include "io/dev2/dev2.h"

typedef struct flop5 flop5_t;

struct flop5 {
	struct em400_dev2 base;

	char *image;
};

em400_dev_t * flop5_create(const char *image);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
