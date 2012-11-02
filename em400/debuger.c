//  Copyright (c) 2012 Jakub Filipowicz <jakubf@gmail.com>
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
#include <ncurses.h>
#include <string.h>

#include "cpu.h"
#include "registers.h"
#include "memory.h"
#include "dasm.h"
#include "errors.h"
#include "utils.h"
#include "debuger.h"
#include "debuger_ui.h"
#include "debuger_parser.h"

int debuger_fin = 0;

struct var_t *variables = NULL;
struct var_t *last_v = NULL;

extern int yyparse();
typedef struct yy_buffer_state *YY_BUFFER_STATE;
YY_BUFFER_STATE yy_scan_string(char *yy_str);
void yy_delete_buffer(YY_BUFFER_STATE b);

// -----------------------------------------------------------------------
void debuger_set_var(char *name, uint16_t value)
{
	struct var_t *v;

	v = debuger_get_var(name);

	if (v) {
		v->value = value;
	} else {
		v = malloc(sizeof(struct var_t));
		v->name = strdup(name);
		v->value = value;
		v->next = NULL;
		if (variables) {
			last_v->next = v;
			last_v = v;
		} else {
			variables = last_v = v;
		}
	}
}

// -----------------------------------------------------------------------
struct var_t * debuger_get_var(char *name)
{
	struct var_t *v = variables;
	while (v) {
		if (!strcmp(name, v->name)) {
			return v;
		}
		v = v->next;
	}
	return NULL;
}

// -----------------------------------------------------------------------
int em400_debuger_init()
{
	nc_w_changed = 1;
	return em400_debuger_ui_init();
}

// -----------------------------------------------------------------------
void em400_debuger_shutdown()
{
	em400_debuger_ui_shutdown();
}

// -----------------------------------------------------------------------
void em400_debuger_loop()
{
	int nbufsize = 70;
	char buf[nbufsize];
	int res;

	debuger_fin = 0;

	while (!debuger_fin) {
		if (nc_w_changed) {
			e400_debuger_w_reinit_all();
			nc_w_changed = 0;
		} else {
			e400_debuger_w_redraw_all();
		}

		res = nc_readline(WCMD, "em400> ", buf, nbufsize);
		waprintw(WCMD, 0, "\n");

		if ((res == KEY_ENTER) && (*buf)) {
			YY_BUFFER_STATE yb = yy_scan_string(buf);
			yyparse();
			yy_delete_buffer(yb);
		}
	}
}

// vim: tabstop=4
