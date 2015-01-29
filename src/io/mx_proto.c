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

#include <inttypes.h>
#include <stdlib.h>

#include "io/mx.h"
#include "io/mx_proto.h"
#include "io/mx_proto_winch.h"

struct mx_proto protos[] = {
	{ /* MX_PROTO_PUNCH_READER */		0, NULL },
	{ /* MX_PROTO_PUNCHER */			0, NULL },
	{ /* MX_PROTO_TERMINAL */			0, NULL },
	{ /* MX_PROTO_SOM_PUNCH_READER */	0, NULL },
	{ /* MX_PROTO_SOM_PUNCHER */		0, NULL },
	{ /* MX_PROTO_SOM_TERMINAL */		0, NULL },
	{ /* MX_PROTO_WINCHESTER */
		sizeof(struct mx_proto_winch),
		mx_proto_winch_init,
		mx_proto_winch_close,
		mx_proto_winch_attach,
		mx_proto_winch_detach,
		mx_proto_winch_transmit
	},
	{ /* MX_PROTO_MTAPE */				0, NULL },
	{ /* MX_PROTO_FLOPPY */				0, NULL },
	{ /* MX_PROTO_TTY_ITWL */			0, NULL }
};

// TODO: check MX_SC_E_DIR_MISMATCH in proto implementations!

// -----------------------------------------------------------------------
int mx_proto_create(int type, uint16_t *args, struct mx_line *lline)
{
	int res;

	if ((type < 0) || (type >= MX_PROTO_MAX) || (protos[type].init == NULL)) {
		return MX_SC_E_PROTO_MISSING;
	}

	// create room for specific protocol
	lline->proto = calloc(1, protos[type].size);
	if (!lline->proto) {
		return MX_SC_E_NOMEM;
	}

	// initialize the protocol
	res = protos[type].init(args, lline);
	if (res != MX_SC_E_OK) {
		return res;
	}

	// set common things
	lline->proto->init = protos[type].init;
	lline->proto->close = protos[type].close;
	lline->proto->attach = protos[type].attach;
	lline->proto->detach = protos[type].detach;
	lline->proto->transmit = protos[type].transmit;

	return MX_SC_E_OK;
}

// -----------------------------------------------------------------------
void mx_proto_destroy(struct mx_proto *proto)
{
	free(proto);
}

// vim: tabstop=4 shiftwidth=4 autoindent
