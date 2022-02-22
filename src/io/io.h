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

#ifndef IO_H
#define IO_H

#include <inttypes.h>
#include <stdbool.h>

#include "cfg.h"

int io_init(em400_cfg *cfg);
void io_shutdown();
void io_reset();
void io_get_intspec(int ch, uint16_t *int_spec);
int io_dispatch(int dir, uint16_t n, uint16_t *r);

void io_int_set(int x);
void io_int_set_pa();
bool io_mem_read_1(int nb, uint16_t addr, uint16_t *data);
bool io_mem_write_1(int nb, uint16_t addr, uint16_t data);
bool io_mem_read_n(int nb, uint16_t saddr, uint16_t *dest, int count);
bool io_mem_write_n(int nb, uint16_t saddr, uint16_t *src, int count);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
