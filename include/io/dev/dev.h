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

typedef int (*dev_callback_f)(unsigned arg);

typedef void * (*dev_create_f)(struct cfg_arg *args);
typedef void (*dev_reset_f)(void *dev);
typedef void (*dev_destroy_f)(void *dev);

typedef unsigned (*dev_get_block_size_f)(void *dev);
typedef int (*dev_is_removable_f)(void *dev);
typedef int (*dev_is_adresable_f)(void *dev);

typedef int (*dev_load_f)(void *dev, const char *image);
typedef int (*dev_eject_f)(void *dev);

typedef int (*dev_read_f)(void *dev, uint8_t *buf, unsigned addres);
typedef int (*dev_write_f)(void *dev, uint8_t *buf, unsigned address, unsigned bytes);

typedef int (*dev_register_callback_f)(void *dev, dev_callback_f callback);

struct dev_drv {
	const char *name;
	dev_create_f create;
	dev_destroy_f destroy;
	dev_reset_f reset;
	dev_read_f read;
	dev_write_f write;
};

int dev_make(struct cfg_unit *dev, const struct dev_drv **dev_drv, void **dev_obj);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
