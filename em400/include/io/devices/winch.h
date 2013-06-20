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

#ifndef WINCH_H
#define WINCH_H

#include <inttypes.h>
#include <stdio.h>

struct winchester_t {
	int cylinders;
	int heads;
	int sectors;
	int sector_size; // bytes
	int total_sectors;
	char *image_name;
	FILE *image;
};

struct winchester_t * winch_create(int cylinders, int heads, int sectors, int sector_size, char *image_name);
void winch_shutdown(struct winchester_t *winch);


int winch_p2l(struct winchester_t *w, int cyl, int head, int sect);
int winch_read_sector_p(struct winchester_t *winch, uint8_t *buf, int cyl, int head, int sect);
int winch_read_sector_l(struct winchester_t *w, uint8_t *buf, int sect);
int winch_write_sector_p(struct winchester_t *winch, uint8_t *buf, int count, int cyl, int head, int sect);
int winch_write_sector_l(struct winchester_t *w, uint8_t *buf, int count, int sect);
int winch_park(int cyl);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
