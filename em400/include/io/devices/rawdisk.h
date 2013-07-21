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

#ifndef RAWDISK_H
#define RAWDISK_H

#include <inttypes.h>
#include <stdio.h>

struct rawdisk_t {
	int cylinders;
	int heads;
	int sectors;
	int sector_size; // bytes
	int total_sectors;
	char *image_name;
	FILE *image;
};

struct rawdisk_t * rawdisk_create(int cylinders, int heads, int sectors, int sector_size, char *image_name);
void rawdisk_shutdown(struct rawdisk_t *d);


int rawdisk_p2l(struct rawdisk_t *d, int cyl, int head, int sect);
int rawdisk_read_sector_p(struct rawdisk_t *d, uint8_t *buf, int cyl, int head, int sect);
int rawdisk_read_sector_l(struct rawdisk_t *d, uint8_t *buf, int sect);
int rawdisk_write_sector_p(struct rawdisk_t *d, uint8_t *buf, int count, int cyl, int head, int sect);
int rawdisk_write_sector_l(struct rawdisk_t *d, uint8_t *buf, int count, int sect);
int rawdisk_park(struct rawdisk_t *d, int cyl);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
