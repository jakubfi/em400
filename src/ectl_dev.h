//  Copyright (c) 2016 Jakub Filipowicz <jakubf@gmail.com>
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

#include <inttypes.h>

#ifndef ECTL_DEV_H
#define ECTL_DEV_H

enum ectl_dev_if_types {
	ECTL_IFTYPE_NONE = 0,
	ECTL_IFTYPE_INT,
	ECTL_IFTYPE_STR,
	ECTL_IFTYPE_ENUM,
};

enum ectl_dev_if_flags {
	ECTL_IFFLAG_NONE	= 0,
	ECTL_IFFLAG_R	= 1 << 0,
	ECTL_IFFLAG_W	= 1 << 1,
	ECTL_IFFLAG_OPTS	= 1 << 2,
};

struct ectl_dev_if_option {
	int val_int;
	char *val_str;
};

struct ectl_dev_if {
	char *id;
	char *name;
	int type;
	uint32_t flags;
	int val_int;
	char *val_str;
	struct ectl_dev_if_option *opts;
};

int ectl_dev_param_int_set(int chan, int unit, int value);
int ectl_dev_param_int_get(int chan, int unit);
int ectl_dev_param_str_set(int chan, int unit, char *value);
char * ectl_dev_param_str_get(int chan, int unit);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
