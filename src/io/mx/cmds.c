//  Copyright (c) 2013-2018 Jakub Filipowicz <jakubf@gmail.com>
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

#include "io/mx/cmds.h"

const char *mx_chan_cmd_names[] = {
	"RESET",
	"INTSPEC",
	"EXISTS",
	"INVALID",
	"[invalid-chan-cmd]"
};

const char *mx_cmd_names[] = {
	"ERR_0",
	"TEST",
	"ATTACH",
	"STATUS",
	"TRANSMIT",
	"SETCFG",
	"ERR_6",
	"ERR_7",
	"ERR_8",
	"REQUEUE",
	"DETACH",
	"ABORT",
	"ERR_C",
	"ERR_D",
	"ERR_E",
	"ERR_F",
	"[invalid-cmd]"
};

// -----------------------------------------------------------------------
const char * mx_get_cmd_name(unsigned i)
{
	if (i < MX_CMD_CNT) {
		return mx_cmd_names[i];
	} else {
		return mx_cmd_names[MX_CMD_CNT];
	}
}

// -----------------------------------------------------------------------
const char * mx_get_chan_cmd_name(unsigned i)
{
	if (i < MX_CHAN_CMD_CNT) {
		return mx_chan_cmd_names[i];
	} else {
		return mx_chan_cmd_names[MX_CHAN_CMD_CNT];
	}
}

// vim: tabstop=4 shiftwidth=4 autoindent
