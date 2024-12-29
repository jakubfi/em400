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

#include <stdlib.h>
#include <inttypes.h>

#include "ectl_dev.h"

struct ectl_dev_if ectl_dev_media[] = {
	{
		.id = "media",
		.name = "Disk image",
		.type = ECTL_IFTYPE_STR,
		.flags = ECTL_IFFLAG_R | ECTL_IFFLAG_W,
	}, {
		.id = "eject",
		.name = "Eject",
		.type = ECTL_IFTYPE_NONE,
		.flags = ECTL_IFFLAG_W,
	}
};

struct ectl_dev_if_option dev_serial_speeds[] = {
	{ 1200, "1200" },
	{ 2400, "2400" },
	{ 4800, "4800" },
	{ 9600, "9600" },
};

struct ectl_dev_if_option dev_serial_params[] = {
	{ 0, "8N1" },
	{ 1, "8N2" },
	{ 2, "8O1" },
	{ 3, "8O2" },
	{ 5, "8E1" },
	{ 6, "8E2" },
};

struct ectl_dev_if dev_serial[] = {
	{
		.id = "speed",
		.name = "Port speed",
		.type = ECTL_IFTYPE_INT,
		.flags = ECTL_IFFLAG_R | ECTL_IFFLAG_W | ECTL_IFFLAG_OPTS,
		.opts = dev_serial_speeds,
	},
	{
		.id = "param",
		.name = "Port parameters",
		.type = ECTL_IFTYPE_INT,
		.flags = ECTL_IFFLAG_R | ECTL_IFFLAG_W | ECTL_IFFLAG_OPTS,
		.opts = dev_serial_params,
	}

};

int ectl_dev_param_int_set(int chan, int unit, int value);
int ectl_dev_param_int_get(int chan, int unit);
int ectl_dev_param_str_set(int chan, int unit, char *value);
char * ectl_dev_param_str_get(int chan, int unit);

// vim: tabstop=4 shiftwidth=4 autoindent
