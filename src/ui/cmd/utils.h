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

char * ui_cmd_skip_ws(char *input);
char * ui_cmd_remove_trailing_ws(char *input);
char * ui_cmd_find_ws(char *input);
char * ui_cmd_cleanup_input(char *input);
char * ui_cmd_gettok_str(char *input, char **token, char **remainder);
int ui_cmd_gettok_int(char *input, char **token, char **remainder);
int ui_cmd_gettok_bool(char *input, char **token, char **remainder);
struct ui_cmd_command * ui_cmd_gettok_cmd(char *input, char **token, char **remainder);

// vim: tabstop=4 shiftwidth=4 autoindent
