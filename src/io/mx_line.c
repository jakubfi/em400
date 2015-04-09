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

#include "log.h"
#include "io/mx_line.h"

const char *mx_phy_type_names[] = {
	"USART async",
	"8255",
	"USART sync",
	"winchester",
	"magnetic tape",
	"floppy",
};

const char *mx_phy_dir_names[] = {
	"[none]",
	"[invalid]",
	"input",
	"[invalid]",
	"output",
	"[invalid]",
	"half-duplex",
	"full-duplex",
};

const char *mx_sc_err_names[] = {
	"configuration already set",
	"wrong number of physical or logical lines",
	"unknown device type in physical line description",
	"wrong transmission direction",
	"incomplete physical line description",
	"protocol not available",
	"physical line is not used",
	"device vs. protocol transmission direction mismatch",
	"physical line is already used",
	"memory exhausted",
	"protocol vs. physical line type mismatch",
	"wrong protocol parameters",
};

static const char *mx_invalid_thing = "[invalid]";

// -----------------------------------------------------------------------
const char * mx_line_dir_name(unsigned i)
{
	if (i < MX_DIR_MAX) {
		return mx_phy_dir_names[i];
	} else {
		return mx_invalid_thing;
	}
}

// -----------------------------------------------------------------------
const char * mx_line_type_name(unsigned i)
{
	if (i < MX_PHY_MAX) {
		return mx_phy_type_names[i];
	} else {
		return mx_invalid_thing;
	}
}

// -----------------------------------------------------------------------
const char * mx_line_sc_err_name(unsigned i)
{
	if (i < MX_SC_E_MAX) {
		return mx_sc_err_names[i];
	} else {
		return mx_invalid_thing;
	}
}

// -----------------------------------------------------------------------
int mx_line_conf_phy(struct mx_line *line, uint16_t data)
{
	unsigned dir  = (data & 0b1110000000000000) >> 13;
	unsigned used = (data & 0b0001000000000000) >> 12;
	unsigned type = (data & 0b0000111100000000) >> 8;

	LOG(L_MX, 3, "    %s (%i), %s (%i), %s", mx_line_type_name(type), type, mx_line_dir_name(dir), dir, used ? "used" : "unused");

	// check type for correctness
	if (type >= MX_PHY_MAX) {
		return MX_SC_E_DEVTYPE;
	// check direction: USART
	} else if ((type == MX_PHY_USART_SYNC) || (type == MX_PHY_USART_ASYNC)) {
		if ((dir != MX_DIR_OUTPUT) && (dir != MX_DIR_INPUT) && (dir != MX_DIR_HALF_DUPLEX) && (dir != MX_DIR_FULL_DUPLEX)) {
			return MX_SC_E_DIR;
		}
	// check direction: 8255
	} else if (type == MX_PHY_8255) {
		if ((dir != MX_DIR_OUTPUT) && (dir != MX_DIR_INPUT)) {
			return MX_SC_E_DIR;
		}
	// check direction: winchester, floppy, tape
	} else {
		if (dir != MX_DIR_NONE) {
			return MX_SC_E_DIR;
		}
	}

	line->dir = dir;
	line->used = used;
	line->type = type;

	return MX_SC_E_OK;
}

// -----------------------------------------------------------------------
int mx_line_conf_log(struct mx_line *line, uint16_t *data)
{
	unsigned proto_num = data[0] >> 8;

	LOG(L_MX, 3, "    Protocol %i: %s", proto_num, mx_proto_name(proto_num));

	// make sure physical line is active (configured)
	if (!line->used) {
		return MX_SC_E_PHY_UNUSED;
	}

	// make sure that no logical line uses this physical line
	if (line->proto) {
		return MX_SC_E_PHY_USED;
	}

	// find protocol
	const struct mx_proto *proto = mx_proto_get(proto_num);
	if (!proto) {
		return MX_SC_E_PROTO_MISSING;
	}

	// check if line direction matches required protocol direction
	if ((proto->dir & line->dir) != proto->dir) {
		return MX_SC_E_DIR_MISMATCH;
	}

	// check if line type is OK for the protocol
	int *type = proto->phy_types;
	while (*type != line->type) {
		if (*type == -1) {
			return MX_SC_E_PROTO_MISMATCH;
		}
		type++;
	}

	// set protocol configuration
	// TODO

	line->proto = proto;

	return MX_SC_E_OK;
}

// vim: tabstop=4 shiftwidth=4 autoindent
