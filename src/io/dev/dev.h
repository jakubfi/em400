//  Copyright (c) 2015 Jakub Filipowicz <jakubf@gmail.com>
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

#ifndef DEV_H
#define DEV_H

#include <inttypes.h>

#include "cfg.h"

enum dev_cmd_status {
	DEV_CMD_OK = 0,
	DEV_CMD_BUSY,
	DEV_CMD_INVALID,
	DEV_CMD_NOMEDIUM,
	DEV_CMD_SEEKERR,
	DEV_CMD_WRPROTECT,
	DEV_CMD_WRERR,
	DEV_CMD_RDERR,
	DEV_CMD_ERR,
};

struct dev_chs {
	unsigned c;
	unsigned h;
	unsigned s;
};

typedef void * (*dev_create_f)(dictionary *cfg, int ch_num, int dev_num);
typedef void (*dev_reset_f)(void *dev);
typedef void (*dev_destroy_f)(void *dev);

typedef int (*dev_sector_rd_f)(void *dev, uint8_t *buf, struct dev_chs *chs);
typedef int (*dev_sector_wr_f)(void *dev, uint8_t *buf, struct dev_chs *chs);
typedef int (*dev_char_rd_f)(void *dev, uint8_t *c);
typedef int (*dev_char_wr_f)(void *dev, uint8_t *c);

struct dev_drv {
	const char *name;
	dev_create_f create;
	dev_destroy_f destroy;
	dev_reset_f reset;
	dev_sector_rd_f sector_rd;
	dev_sector_wr_f sector_wr;
	dev_char_rd_f char_rd;
	dev_char_wr_f char_wr;
};

int dev_make(dictionary *cfg, int ch_num, int dev_num, const struct dev_drv **dev_drv, void **dev_obj);
void dev_chs_next(struct dev_chs *chs, unsigned heads, unsigned spt);
void dev_lba2chs(unsigned lba, struct dev_chs *chs, unsigned heads, unsigned spt);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
