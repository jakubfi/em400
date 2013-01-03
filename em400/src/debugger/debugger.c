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

int ui_mode = O_NCURSES;
//int ui_mode = O_STD;

// debuger flow
int dbg_loop_fin = 0;
volatile int dbg_enter = 1;

// store user input here
char input_buf[INPUT_BUF_SIZE];

// breakpoints
struct break_t *brk_stack = NULL;
struct break_t *brk_last = NULL;

// trace memory activity
int mem_act_block;
int mem_actr_min;
int mem_actr_max = -1;
int mem_actw_min;
int mem_actw_max = -1;

// trace register activity
int reg_act[R_MAX];

extern int yyparse();
typedef struct yy_buffer_state *YY_BUFFER_STATE;
YY_BUFFER_STATE yy_scan_string(char *yy_str);
void yy_delete_buffer(YY_BUFFER_STATE b);

// -----------------------------------------------------------------------
void _dbg_sigint_handler(int signum, siginfo_t *si, void *ctx)
{
	dbg_enter = 1;
}

// -----------------------------------------------------------------------
int dbg_init()
{
	// set UI mode
	if (aw_init(ui_mode)) {
		return -1;
	}

	dbg_ui_init();
	aw_layout_changed = 1;

	// prepare handler for ctrl-c (break emulation, enter debugger loop)
	struct sigaction sa;
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = _dbg_sigint_handler;

	if (sigemptyset(&sa.sa_mask) != 0) {
		return -1;
	}

	if (sigaction(SIGINT, &sa, NULL) != 0) {
		return -1;
	}

	// register action is none when debugger starts
	for (int i=0 ; i<R_MAX ; i++) {
		reg_act[i] = C_DATA;
	}

	return 0;
}

// -----------------------------------------------------------------------
void dbg_shutdown()
{
	aw_shutdown();
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
    // we're leaving debuger, clear memory/register traces
    mem_actr_max = -1;
    mem_actw_max = -1;
    for (int r=0 ; r<R_MAX ; r++) {
        reg_act[r] = C_DATA;
    }
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
			YY_BUFFER_STATE yb = yy_scan_string(input_buf);
			yyparse();
			yy_delete_buffer(yb);
		}
	}
	dbg_fin_cycle();
}

// vim: tabstop=4
