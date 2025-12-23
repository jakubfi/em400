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

#ifndef SP45DE_H
#define SP45DE_H

#include "io/dev2/dev2.h"

typedef struct sp45de sp45de_t;

struct sp45de {
	struct em400_dev2 base;

	char *images[4];
};

em400_dev_t * sp45de_create(const char *images[4]);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
