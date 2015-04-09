//  Copyright (c) 2013-2015 Jakub Filipowicz <jakubf@gmail.com>
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

#ifndef MX_PROTO_H
#define MX_PROTO_H

#include <inttypes.h>

enum mx_protocols {
	MX_PROTO_PUNCH_READER		= 0,
	MX_PROTO_PUNCHER			= 1,
	MX_PROTO_TERMINAL			= 2,
	MX_PROTO_SOM_PUNCH_READER	= 3,
	MX_PROTO_SOM_PUNCHER		= 4,
	MX_PROTO_SOM_TERMINAL		= 5,
	MX_PROTO_WINCHESTER			= 6,
	MX_PROTO_MTAPE				= 7,
	MX_PROTO_FLOPPY				= 8,
	MX_PROTO_TTY_ITWL			= 9, // telex for ITWL?
	MX_PROTO_MAX
};

struct mx_line;

typedef void (*proto_conf_f)(struct mx_line *line, uint16_t *data);

struct mx_proto {
	int enabled;
	char *name;
	uint16_t dir;
	int *phy_types;
};

const char * mx_proto_name(unsigned i);
const struct mx_proto * mx_proto_get(unsigned i);


#endif

// vim: tabstop=4 shiftwidth=4 autoindent
