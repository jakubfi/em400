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
#include <string.h>

#include "cpu.h"
#include "registers.h"
#include "memory.h"
#include "dasm.h"
#include "errors.h"
#include "utils.h"
#include "awin.h"
#include "debuger.h"
#include "debuger_ui.h"
#include "debuger_parser.h"
#include "debuger_eval.h"

int ui_mode = O_NCURSES;

volatile int debuger_loop_fin = 0;
volatile int debuger_enter = 1;

char input_buf[INPUT_BUF_SIZE];

struct var_t *variables = NULL;
struct var_t *last_v = NULL;

struct break_t *brkpoints = NULL;

extern int yyparse();
typedef struct yy_buffer_state *YY_BUFFER_STATE;
YY_BUFFER_STATE yy_scan_string(char *yy_str);
void yy_delete_buffer(YY_BUFFER_STATE b);

// -----------------------------------------------------------------------
void _debuger_sigint_handler(int signum, siginfo_t *si, void *ctx)
{
	debuger_enter = 1;
}

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
	if (aw_init(ui_mode)) {
		return -1;
	}

	em400_debuger_ui_init();
	aw_layout_changed = 1;

    // prepare handler for ctrl-c
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = _debuger_sigint_handler;

    if (sigemptyset(&sa.sa_mask) != 0) {
        return -1;
    }

    if (sigaction(SIGINT, &sa, NULL) != 0) {
        return -1;
    }

	return 0;
}

// -----------------------------------------------------------------------
void em400_debuger_shutdown()
{
	aw_shutdown();
}

// -----------------------------------------------------------------------
struct break_t * em400_debuger_brk_check()
{
	struct break_t *b = brkpoints;
	while (b) {
		if ((!b->disabled) && (n_eval(b->n))) {
			b->counter++;
			return b;
		}
		b = b->next;
	}
	return NULL;
}

// -----------------------------------------------------------------------
void em400_debuger_step()
{
	struct break_t *bhit = NULL;

	if ((!debuger_enter) && (!(bhit=em400_debuger_brk_check()))) {
		return;
	}

	if (bhit) {
		awprint(W_CMD, C_LABEL, "Hit breakpoint ");
		awprint(W_CMD, C_DATA, "%i", bhit->nr);
		awprint(W_CMD, C_LABEL, ": \"");
		awprint(W_CMD, C_DATA, "%s", bhit->label);
		awprint(W_CMD, C_LABEL, "\" (cnt: ");
		awprint(W_CMD, C_DATA, "%i", bhit->counter);
		awprint(W_CMD, C_LABEL, ")\n");
	}

	debuger_loop_fin = 0;

	while (!debuger_loop_fin) {
		if (aw_layout_changed) {
			aw_layout_redo();
		} else {
			aw_layout_refresh();
		}

		int res = aw_readline(W_CMD, C_PROMPT, "em400> ", input_buf, INPUT_BUF_SIZE);
		if (ui_mode == O_NCURSES) {
			awprint(W_CMD, C_LABEL, "\n");
		}

		aw_layout_refresh();

		if ((res == KEY_ENTER) && (*input_buf)) {
			YY_BUFFER_STATE yb = yy_scan_string(input_buf);
			yyparse();
			yy_delete_buffer(yb);
		}
	}
}

// vim: tabstop=4
