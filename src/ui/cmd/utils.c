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

#include <ctype.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>

#include "ui/cmd/commands.h"

// -----------------------------------------------------------------------
char * ui_cmd_skip_ws(char *input)
{
	while (input && isspace(*input)) {
		input++;
	}

	return input;
}

// -----------------------------------------------------------------------
char * ui_cmd_find_ws(char *input)
{
	while (input && *input && !isspace(*input)) {
		input++;
	}
	if (!*input) {
		return NULL;
	} else {
		return input;
	}
}

// -----------------------------------------------------------------------
char * ui_cmd_remove_trailing_ws(char *input)
{
	if (!input) return NULL;

    char *input_end = input + strlen(input)-1;
	while (input_end && isspace(*input_end)) {
		*input_end = '\0';
		input_end--;
	}
	return input;
}

// -----------------------------------------------------------------------
char * ui_cmd_gettok_str(char *input, char **token, char **remainder)
{
	// skip whitespace
	*token = ui_cmd_skip_ws(input);

	if (!**token) {
		*token = NULL;
		*remainder = NULL;
	} else {
		// find token end
		*remainder = ui_cmd_find_ws(*token);
		if (*remainder) {
			**remainder = '\0';
			(*remainder)++;
		}
	}

	return *token;
}

// -----------------------------------------------------------------------
int ui_cmd_gettok_int(char *input, char **token, char **remainder)
{
	ui_cmd_gettok_str(input, token, remainder);

	if (!*token) {
		return -1;
	}

	char *strerr;
	long value = strtol(*token, &strerr, 0);

	if ((value < -32768) || (value > 0xffff) || (*strerr != '\0')) {
		return -1;
	}
	return value & 0xffff;
}

// -----------------------------------------------------------------------
int ui_cmd_gettok_bool(char *input, char **token, char **remainder)
{
	ui_cmd_gettok_str(input, token, remainder);
	if (!*token) {
		return -1;
	} else if (!strcasecmp(*token, "on")) {
		return 1;
	} else if (!strcasecmp(*token, "off")) {
		return 0;
	} else {
		return -1;
	}
}

// -----------------------------------------------------------------------
struct ui_cmd_command * ui_cmd_gettok_cmd(char *input, char **token, char **remainder)
{
	ui_cmd_gettok_str(input, token, remainder);
	return ui_cmd_find_command(*token);
}


// vim: tabstop=4 shiftwidth=4 autoindent
