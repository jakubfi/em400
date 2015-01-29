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

#ifndef MX_PROTO_H
#define MX_PROTO_H

#include <inttypes.h>

struct mx_proto;

// -----------------------------------------------------------------------
// logical line protocols
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
	MX_PROTO_TTY_ITWL			= 9, // teleks ?
	MX_PROTO_MAX
};

typedef int (*proto_init_f)(uint16_t *args, struct mx_line *lline);
typedef void (*proto_close_f)(struct mx_proto *proto);
typedef int (*proto_attach_f)(struct mx_line *line, uint16_t *arg);
typedef void (*proto_detach_f)(struct mx_line *line);
typedef void (*proto_transmit_f)(struct mx_proto *proto, uint16_t *arg);

struct mx_proto {
	int size;
	proto_init_f init;
	proto_close_f close;
	proto_attach_f attach;
	proto_detach_f detach;
	proto_transmit_f transmit;
};

extern int mx_proto_create(int type, uint16_t *args, struct mx_line *lline);
extern void mx_proto_destroy(struct mx_proto *proto);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
