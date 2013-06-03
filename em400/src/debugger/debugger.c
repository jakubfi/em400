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
#include <limits.h>

#include "cpu/cpu.h"
#include "cpu/registers.h"
#include "cpu/memory.h"

#include "em400.h"
#include "cfg.h"
#include "errors.h"
#include "utils.h"

#include "debugger/dasm.h"
#include "debugger/awin.h"
#include "debugger/debugger.h"
#include "debugger/ui.h"
#include "debugger/parser.h"
#include "debugger/eval.h"

char *script_name = NULL;

int ui_mode;

// debugger flow
int dbg_loop_fin = 0;
volatile int dbg_enter = 1;

// store user input here
char input_buf[INPUT_BUF_SIZE];

// mem/reg/int touches
struct touch_t *touch_mem = NULL;
struct touch_t *touch_reg = NULL;
struct touch_t *touch_int = NULL;

// breakpoints
struct evlb_t *brk_stack = NULL;
struct evlb_t *brk_top = NULL;

// watches
struct evlb_t *watch_stack = NULL;
struct evlb_t *watch_top = NULL;

extern int yyparse();
typedef struct yy_buffer_state *YY_BUFFER_STATE;
YY_BUFFER_STATE yy_scan_string(char *yy_str);
void yy_delete_buffer(YY_BUFFER_STATE b);

// -----------------------------------------------------------------------
void dbg_touch_add(struct touch_t **t, int type, int block, int pos, int oval)
{
	struct touch_t *nt = dbg_touch_get(t, block, pos);
	if (nt) {
		nt->type |= type;
	} else {
		nt = malloc(sizeof(struct touch_t));
		nt->type = type;
		nt->block = block;
		nt->pos = pos;
		nt->oval = oval;
		nt->next = *t;
		*t = nt;
	}
}

// -----------------------------------------------------------------------
struct touch_t * dbg_touch_get(struct touch_t **t, int block, int pos)
{
	struct touch_t *tf = *t;
	while (tf) {
		if ((tf->block == block) && (tf->pos == pos)) {
			return tf;
		}
		tf = tf->next;
	}
	return NULL;
}

// -----------------------------------------------------------------------
int dbg_touch_check(struct touch_t **t, int block, int pos)
{
	struct touch_t *tf = dbg_touch_get(t, block, pos);
	if (tf) {
		return tf->type;
	} else {
		return 0;
	}
}

// -----------------------------------------------------------------------
void dbg_touch_pop(struct touch_t **t)
{
	if (!*t) return;
	struct touch_t *tn = (*t)->next;
	free(*t);
	*t = tn;
}

// -----------------------------------------------------------------------
void dbg_touch_drop_all(struct touch_t **t)
{
	if (!*t) return;
	dbg_touch_drop_all(&(*t)->next);
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
	eprint("Initializing debugger: ");
	// set UI mode
	if (em400_cfg.ui_simple == 1) {
		eprint("simple\n");
		ui_mode = O_STD;
	} else {
		eprint("ncurses\n");
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
	eprint("Shutdown debugger\n");
	aw_shutdown();
}

// -----------------------------------------------------------------------
struct evlb_t * dbg_brk_check()
{
	struct evlb_t *b = brk_stack;
	while (b) {
		if ((!b->disabled) && (n_eval(b->n))) {
			awprint(W_CMD, C_LABEL, "Hit breakpoint ");
			awprint(W_CMD, C_DATA, "%i", b->nr);
			awprint(W_CMD, C_LABEL, ": \"");
			awprint(W_CMD, C_DATA, "%s", b->label);
			awprint(W_CMD, C_LABEL, "\" (at: ");
			awprint(W_CMD, C_DATA, "0x%04x", nR(R_IC));
			awprint(W_CMD, C_LABEL, ", cnt: ");
			awprint(W_CMD, C_DATA, "%i", b->value);
			awprint(W_CMD, C_LABEL, ")\n");
			b->value++;
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
	dbg_touch_drop_all(&touch_mem);
	dbg_touch_drop_all(&touch_reg);
}

// -----------------------------------------------------------------------
int dbg_parse(char *c)
{
	YY_BUFFER_STATE yb = yy_scan_string(c);
	int res = yyparse();
	yy_delete_buffer(yb);
	return res;
}

// -----------------------------------------------------------------------
int read_script(char *filename)
{
	FILE *sf = fopen(filename, "r");
	if (!sf) {
		return INT_MIN;;
	}

	char *b = input_buf;
	int lines = 0;
	int linebeg = 1;
	int comment = 0;

	while (1) {
		int c = fread(b, 1, 1, sf);

		if (c <= 0) {
			break;
		}

		// skip leading blanks
		if (((*b == ' ') || (*b == '\t')) && (linebeg)) {
		// comment start
		} else if (*b == ';') {
			comment = 1;
		// skip comments
		} else if (comment && (*b != '\n')) {
		// EOL, do parse
		} else if (*b == '\n') {
			linebeg = 1;
			comment = 0;
			*(b+1) = '\0';
			*b = '\0';
			if (b != input_buf) {
				awprint(W_CMD, C_LABEL, "%s ", input_buf);
				awprint(W_CMD, C_PROMPT, "-> ", input_buf);
				*b = '\n';
				int res = dbg_parse(input_buf);
				if (res != 0) {
					return -(lines+1);
				}
				b = input_buf;
				lines++;
			}
		// next char
		} else {
			linebeg = 0;
			b++;
		}
	}

	fclose(sf);
	return lines;
}

// -----------------------------------------------------------------------
void dbg_step()
{
	struct evlb_t *bhit = NULL;

	if ((!dbg_enter) && (!(bhit=dbg_brk_check()))) {
		dbg_fin_cycle();
		return;
	}

	dbg_loop_fin = 0;

	while (!dbg_loop_fin) {
		if (aw_layout_changed) {
			awin_tb_scroll_end(1);
			aw_layout_redo();
		} else {
			aw_layout_refresh();
		}

		if (script_name) {
			int sr = read_script(script_name);
			if (sr == INT_MIN) {
				awprint(W_CMD, C_ERROR, "Cannot open script file: %s\n", script_name);
			} else if (sr<0) {
				awprint(W_CMD, C_ERROR, "Error at line: %i\n", -sr);
			} else {
				awprint(W_CMD, C_LABEL, "Read %i line(s)\n", sr);
			}
			free(script_name);
			script_name = NULL;
		}

		aw_layout_refresh();

		int res = aw_readline(W_CMD, C_PROMPT, "em400> ", input_buf, INPUT_BUF_SIZE);
		if (ui_mode == O_NCURSES) {
			awprint(W_CMD, C_LABEL, "\n");
		}

		if ((res == KEY_ENTER) && (*input_buf)) {
			awin_tb_scroll_end(1);
			int len = strlen(input_buf);
			input_buf[len] = '\n';
			input_buf[len+1] = '\0';
			dbg_parse(input_buf);
		} else if (res == KEY_NPAGE) {
			awin_tb_scroll(1, 10);
		} else if (res == KEY_PPAGE) {
			awin_tb_scroll(1, -10);
		}

	}
	dbg_fin_cycle();
}


// vim: tabstop=4
