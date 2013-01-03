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

#ifndef DRV_UNONE_H
#define DRV_UNONE_H

#include "io.h"

int drv_unone_init(struct unit_t *u, int cfgc, char **cfgv);
void drv_unone_shutdown(struct unit_t *u);
void drv_unone_reset(struct unit_t *u);
int drv_unone_cmd(struct unit_t *u, int dir, int cmd, int r);

#endif

// vim: tabstop=4
