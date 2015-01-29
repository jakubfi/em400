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

#define PROTO ((struct mx_proto_winch *) proto)

// -----------------------------------------------------------------------
int mx_proto_winch_init(uint16_t *args, struct mx_line *lline)
{
	// at this point line protocol is attached,
	// we only need to check things and set
	// protocol-specific parameters

	if (lline->type != MX_PHY_WINCHESTER) {
		return MX_SC_E_PROTO_MISMATCH;
	}

	if (lline->dir != MX_DIR_WINCH) {
		return MX_SC_E_DIR_MISMATCH;
	}

	struct mx_proto *proto = lline->proto;

	PROTO->newmx	= (args[0] & 0b0000100000000000) >> 11;
	PROTO->heads	= (args[0] & 0b0000011100000000) >> 8;
	PROTO->fprotect	= (args[0] & 0b0000000011111111);

	return MX_SC_E_OK;
}

// -----------------------------------------------------------------------
void mx_proto_winch_close(struct mx_proto *proto)
{

}

// -----------------------------------------------------------------------
int mx_proto_winch_attach(struct mx_line *line, uint16_t *arg)
{
	return 0;
}

// -----------------------------------------------------------------------
void mx_proto_winch_detach(struct mx_line *line)
{

}

// -----------------------------------------------------------------------
void mx_proto_winch_transmit(struct mx_proto *proto, uint16_t *arg)
{

}

// vim: tabstop=4 shiftwidth=4 autoindent
