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

#include <inttypes.h>
#include <stdio.h>

#include "io/dev2/dev2.h"

#define SP45DE_SLOT_COUNT 4
#define SP45DE_TRACK_CNT 77
#define SP45DE_TRACK_LAST 73
#define SP45DE_SECTOR_PER_TRACK 26
#define SP45DE_BLK_SIZE 128

enum sp45de_buf_state {
	SP45DE_BUF_OK, SP45DE_BUF_END
};

typedef struct sp45de sp45de_t;

struct sp45de {
	struct em400_dev2 base;

	char *image_name[4];
	uint8_t buf[SP45DE_BLK_SIZE];
	bool locked[SP45DE_BLK_SIZE];
	FILE *image[SP45DE_BLK_SIZE];
	unsigned buf_pos;
};

em400_dev_t * sp45de_create(const char *image_name[4]);
int sp45de_blk_read(sp45de_t *sp45de, unsigned slot, unsigned track, unsigned sector);
int sp45de_blk_write(sp45de_t *sp45de, unsigned slot, unsigned track, unsigned sector);
int sp45de_read(sp45de_t *sp45de, uint8_t *c);
int sp45de_write(sp45de_t *sp45de, uint8_t c);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
