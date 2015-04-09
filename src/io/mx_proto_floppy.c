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

#include <inttypes.h>

#include "log.h"
#include "io/mx_line.h"
#include "io/mx_proto.h"

// -----------------------------------------------------------------------
int mx_proto_floppy_phy_types[] = { MX_PHY_FLOPPY, -1 };

struct mx_proto mx_proto_floppy = {
	1,
	"floppy 5.25\"",
	MX_DIR_NONE,
	mx_proto_floppy_phy_types,
};

// vim: tabstop=4 shiftwidth=4 autoindent
