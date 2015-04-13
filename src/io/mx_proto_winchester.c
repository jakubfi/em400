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
#include "io/mx_line.h"
#include "io/mx_proto.h"

struct proto_winchester_data {
	int heads;
	int fprotect;
	int wide_sector_addr;
};

// -----------------------------------------------------------------------
int mx_proto_winchester_conf(struct mx_line *line, uint16_t *data)
{
	struct proto_winchester_data *proto_data = malloc(sizeof(struct proto_winchester_data));
	if (!proto_data) {
		return MX_SC_E_NOMEM;
	}

	proto_data->wide_sector_addr	=  (data[0] & 0b0000100000000000) >> 11;
	proto_data->heads				= ((data[0] & 0b0000011100000000) >> 8) + 1;
	proto_data->fprotect			=  (data[0] & 0b0000000011111111);

	LOGID(L_WNCH, 3, line, "    Winchester drive: %i heads, %s sector address%s",
		proto_data->heads,
		proto_data->wide_sector_addr ? "long" : "short",
		proto_data->fprotect ? ", format-protected" : ""
	);

	line->proto_data = proto_data;

	return MX_SC_E_OK;
}

// -----------------------------------------------------------------------
void mx_proto_winchester_free(struct mx_line *line)
{
	free(line->proto_data);
	line->proto_data = NULL;
}

// -----------------------------------------------------------------------
int mx_proto_winchester_phy_types[] = { MX_PHY_WINCHESTER, -1 };

struct mx_proto mx_proto_winchester = {
	.enabled = 1,
	.name = "winchester",
	.dir = MX_DIR_NONE,
	.phy_types = mx_proto_winchester_phy_types,
	.conf = mx_proto_winchester_conf,
	.free = mx_proto_winchester_free,
};

// vim: tabstop=4 shiftwidth=4 autoindent
