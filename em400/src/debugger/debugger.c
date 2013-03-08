//  Copyright (c) 2012-2013 Jakub Filipowicz <jakubf@gmail.com>
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

#include "em400.h"
#include "cpu.h"
#include "registers.h"
#include "memory.h"
#include "errors.h"
#include "utils.h"

#include "debugger/dasm.h"
#include "debugger/awin.h"
#include "debugger/debugger.h"
#include "debugger/ui.h"
#include "parser.h"
#include "debugger/eval.h"

int ui_mode;

// debugger flow
int dbg_loop_fin = 0;
volatile int dbg_enter = 1;

// store user input here
char input_buf[INPUT_BUF_SIZE];

// mem/reg touches
struct touch_t *touch_mem = NULL;
struct touch_t *touch_reg = NULL;

// breakpoints
struct break_t *brk_stack = NULL;
struct break_t *brk_last = NULL;

// trace interrupts activity
uint32_t int_act;
int int_serve_stack[32];
int int_serve_top;

extern int yyparse();
typedef struct yy_buffer_state *YY_BUFFER_STATE;
YY_BUFFER_STATE yy_scan_string(char *yy_str);
void yy_delete_buffer(YY_BUFFER_STATE b);

// -----------------------------------------------------------------------
void dbg_touch(struct touch_t **t, int type, int block, int pos)
{
	struct touch_t *nt = dbg_touch_get(t, block, pos);
	if (nt) {
		nt->type |= type;
	} else {
		nt = malloc(sizeof(struct touch_t));
		nt->type = type;
		nt->block = block;
		nt->pos = pos;
		nt->next = *t;
		*t = nt;
	}
}

// -----------------------------------------------------------------------
struct touch_t * dbg_touch_get(struct touch_t **t, int block, int pos)
{
	if (!*t) return NULL;
	if (((*t)->block == block) && ((*t)->pos == pos)) return *t;
	return dbg_touch_get(&(*t)->next, block, pos);
}

// -----------------------------------------------------------------------
int dbg_touch_check(struct touch_t **t, int block, int pos)
{
	if (!*t) return 0;
	if (((*t)->block == block) && ((*t)->pos == pos)) return (*t)->type;
	return dbg_touch_check(&(*t)->next, block, pos);
}

// -----------------------------------------------------------------------
void dbg_drop_touches(struct touch_t **t)
{
	if (!*t) return;
	dbg_drop_touches(&(*t)->next);
	free(*t);
	*t = NULL;
}

// -----------------------------------------------------------------------
int dbg_touch2attr(int t)
{
	if (t == 3) return C_RW;
	else if (t == 2) return C_WRITE;
	else if (t == 1) return C_READ;
	else return C_DATA;
}

// -----------------------------------------------------------------------
void _dbg_sigint_handler(int signum, siginfo_t *si, void *ctx)
{
	dbg_enter = 1;
}

// -----------------------------------------------------------------------
int dbg_init()
{
	// set UI mode
	if (em400_opts.ui_simple == 1) {
		ui_mode = O_STD;
	} else {
		ui_mode = O_NCURSES;
	}

	if (aw_init(ui_mode) != 0) {
		return E_AW_INIT;
	}

	if ((ui_mode == O_NCURSES) &&  (dbg_ui_init() != 0)) {
		return E_UI_INIT;
	}

	aw_layout_changed = 1;

	// prepare handler for ctrl-c (break emulation, enter debugger loop)
	struct sigaction sa;
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = _dbg_sigint_handler;

	if (sigemptyset(&sa.sa_mask) != 0) {
		return E_UI_SIG_CTRLC;
	}

	if (sigaction(SIGINT, &sa, NULL) != 0) {
		return E_UI_SIG_CTRLC;
	}

	// register/memory action is none when debugger starts
	dbg_fin_cycle();

	return 0;
}

// -----------------------------------------------------------------------
void dbg_shutdown()
{
	if (ui_mode == O_NCURSES) {
		aw_shutdown();
	}
}

// -----------------------------------------------------------------------
struct break_t * dbg_brk_check()
{
	struct break_t *b = brk_stack;
	while (b) {
		if ((!b->disabled) && (n_eval(b->n))) {
			awprint(W_CMD, C_LABEL, "Hit breakpoint ");
			awprint(W_CMD, C_DATA, "%i", b->nr);
			awprint(W_CMD, C_LABEL, ": \"");
			awprint(W_CMD, C_DATA, "%s", b->label);
			awprint(W_CMD, C_LABEL, "\" (cnt: ");
			awprint(W_CMD, C_DATA, "%i", b->counter);
			awprint(W_CMD, C_LABEL, ")\n");
			b->counter++;
			return b;
		}
		b = b->next;
	}
	return NULL;
}

// -----------------------------------------------------------------------
void dbg_fin_cycle()
{
    // we're leaving debugger, clear memory/register traces
	dbg_drop_touches(&touch_mem);
	dbg_drop_touches(&touch_reg);
}

// -----------------------------------------------------------------------
void dbg_parse(char *c)
{
	YY_BUFFER_STATE yb = yy_scan_string(c);
	yyparse();
	yy_delete_buffer(yb);
}

// -----------------------------------------------------------------------
void dbg_step()
{
	struct break_t *bhit = NULL;

	if ((!dbg_enter) && (!(bhit=dbg_brk_check()))) {
		dbg_fin_cycle();
		return;
	}

	dbg_loop_fin = 0;

	while (!dbg_loop_fin) {
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
			dbg_parse(input_buf);
		}
	}
	dbg_fin_cycle();
}

// vim: tabstop=4
