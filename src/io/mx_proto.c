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

#include <stdlib.h>
#include <inttypes.h>

#include "log.h"
#include "io/mx_proto.h"

extern struct mx_proto mx_proto_punchreader;
extern struct mx_proto mx_proto_puncher;
extern struct mx_proto mx_proto_terminal;
extern struct mx_proto mx_proto_som_punchreader;
extern struct mx_proto mx_proto_som_puncher;
extern struct mx_proto mx_proto_som_terminal;
extern struct mx_proto mx_proto_winchester;
extern struct mx_proto mx_proto_tape;
extern struct mx_proto mx_proto_floppy;
extern struct mx_proto mx_proto_ttyitwl;

const struct mx_proto * mx_protocols[] = {
	&mx_proto_punchreader,
	&mx_proto_puncher,
	&mx_proto_terminal,
	&mx_proto_som_punchreader,
	&mx_proto_som_puncher,
	&mx_proto_som_terminal,
	&mx_proto_winchester,
	&mx_proto_tape,
	&mx_proto_floppy,
	&mx_proto_ttyitwl,
};

// -----------------------------------------------------------------------
const struct mx_proto * mx_proto_get(unsigned i)
{
	if ((i < MX_PROTO_MAX) && mx_protocols[i]->enabled) {
		return mx_protocols[i];
	} else {
		return NULL;
	}
}

// vim: tabstop=4 shiftwidth=4 autoindent
