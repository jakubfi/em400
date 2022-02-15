//  Copyright (c) 2016 Jakub Filipowicz <jakubf@gmail.com>
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

#include <stdio.h>

typedef void (*ui_cmd_f)(FILE *out, char *args);

struct ui_cmd_command {
	unsigned flags;
	const char *name;
	const char *args;
	const char *desc;
	ui_cmd_f fun;
};

enum ui_cmd_flags {
	UI_CMD_FLAG_NONE		= 0,
	UI_CMD_FLAG_QUIT		= 0x1,
};
enum ui_cmd_response_states { RESP_OK, RESP_ERR };
enum ui_cmd_eol { UI_NOEOL, UI_EOL };

void ui_cmd_resp(FILE *out, int status, int eol, const char *fmt, ...);
struct ui_cmd_command * ui_cmd_find_command(const char *name);

// vim: tabstop=4 shiftwidth=4 autoindent
