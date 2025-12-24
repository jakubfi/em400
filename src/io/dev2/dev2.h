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

#ifndef DEV2_H
#define DEV2_H

enum em400_device_types {
	EM400_DEV_TERMINAL,
	EM400_DEV_SP45DE,
	EM400_DEV_WINCHESTER,
	EM400_DEV_FLOP5,
};

typedef struct em400_dev2 em400_dev_t;

typedef void (*dev2_noarg_f)(em400_dev_t *dev);
typedef int (*dev2_write_f)(em400_dev_t *dev, char c);

struct em400_dev2 {
	int type;
	char *name;

	dev2_noarg_f reset;
	dev2_noarg_f shutdown;
	dev2_noarg_f free;
	dev2_write_f write;
};

#endif
