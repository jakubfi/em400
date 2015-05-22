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

#include "io/mx_task.h"

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
	MX_PROTO_MAX
};

struct mx_line;

typedef int (*proto_conf_f)(struct mx_line *line, uint16_t *data);
typedef void (*proto_free_f)(struct mx_line *line);
typedef uint8_t (*proto_task_f)(struct mx_line *line, int *irq, uint16_t *data);

struct mx_proto_task {
						// input data field offset is always 0
	int input_flen;		// input data field length (0 if none)
	int output_fpos;	// output data field offset
	int output_flen;	// output data field length (0 if none)
	proto_task_f fun[8];// for each condition, there is separate handler function.
						// NOTE: it is common that not all conditions have handlers.
};

struct mx_proto {
	int enabled;
	char *name;
	uint16_t dir;
	int *phy_types;
	proto_conf_f conf;
	proto_free_f free;
	struct mx_proto_task task[MX_TASK_MAX]; // for each task, there is separate mx_proto_task
};

const struct mx_proto * mx_proto_get(unsigned i);


#endif

// vim: tabstop=4 shiftwidth=4 autoindent
