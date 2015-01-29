//  Copyright (c) 2014 Jakub Filipowicz <jakubf@gmail.com>
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

#ifndef DEV_WINCH_H
#define DEV_WINCH_H

#include <pthread.h>

#include "cfg.h"
#include "io/dev/e4image.h"
#include "io/dev.h"

struct dev_winch {
	struct dev device;
	struct e4i_t *winchester;
};

extern int dev_winch_setup(struct dev *device, struct cfg_arg *args);
extern void dev_winch_close(struct dev *device);
extern void dev_winch_reset(struct dev *device);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
