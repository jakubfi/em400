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
#include <pthread.h>
#include <signal.h>
#include <emdas.h>

#include "ectl.h"

#include "ui/curses/awin.h"
#include "ui/curses/debugger.h"
#include "ui/curses/ui.h"
#include "ui/curses/eval.h"
#include "debugger_parser.h"

int ui_mode;

// debugger flow
int dbg_loop_fin = 0;
volatile int dbg_enter = 1;

// store user input here
char input_buf[INPUT_BUF_SIZE];

// breakpoints
struct evlb_t *brk_stack = NULL;
struct evlb_t *brk_top = NULL;

// watches
struct evlb_t *watch_stack = NULL;
struct evlb_t *watch_top = NULL;

// emdas
struct emdas *emd;

extern int yyparse();
typedef struct yy_buffer_state *YY_BUFFER_STATE;
YY_BUFFER_STATE yy_scan_string(char *yy_str);
void yy_delete_buffer(YY_BUFFER_STATE b);

// -----------------------------------------------------------------------
static void _dbg_sigint_handler(int signum)
{
	signal(SIGINT, _dbg_sigint_handler);
	dbg_enter = 1;
}

// -----------------------------------------------------------------------
int dbg_mem_get(int nb, uint16_t addr, uint16_t *data)
{
	return ectl_mem_get(nb, addr, data, 1);
}

// -----------------------------------------------------------------------
int dbg_init(struct cfg_em400 *cfg)
{
	// set UI mode
	if (cfg->ui_simple == 1) {
		ui_mode = O_STD;
	} else {
		ui_mode = O_NCURSES;
	}

	const char *hfile = "/.em400/history";
	char *home = getenv("HOME");
	char *hist_file = (char *) malloc(strlen(home) + strlen(hfile) + 1);
	sprintf(hist_file, "%s%s", home, hfile);

	if (aw_init(ui_mode, hist_file) != 0) {
		free(hist_file);
		return -1;
	}

	free(hist_file);

	if ((ui_mode == O_NCURSES) &&  (dbg_ui_init() != 0)) {
		return -1;
	}

	aw_layout_changed = 1;

	// prepare handler for ctrl-c (break emulation, enter debugger loop)
	if (signal(SIGINT, _dbg_sigint_handler) == SIG_ERR) {
		return -1;
	}

	emd = emdas_create(cfg->cpu_mod ? EMD_ISET_MX16 : EMD_ISET_MERA400, dbg_mem_get);
	if (!emd) {
		return -1;
	}

	emdas_set_nl(emd, '\0');
	emdas_set_features(emd, EMD_FEAT_NONE);
	emdas_set_tabs(emd, 0, 0, 4, 4);

	return 0;
}

// -----------------------------------------------------------------------
void dbg_shutdown()
{
	aw_shutdown();
	emdas_destroy(emd);
}

// -----------------------------------------------------------------------
struct evlb_t * dbg_brk_check()
{
	struct evlb_t *b = brk_stack;
	while (b) {
		if ((!b->disabled) && (n_eval(b->n))) {
			awtbprint(W_CMD, C_LABEL, "Hit breakpoint ");
			awtbprint(W_CMD, C_DATA, "%i", b->nr);
			awtbprint(W_CMD, C_LABEL, ": \"");
			awtbprint(W_CMD, C_DATA, "%s", b->label);
			awtbprint(W_CMD, C_LABEL, "\" (at: ");
			awtbprint(W_CMD, C_DATA, "0x%04x", ectl_reg_get(ECTL_REG_IC));
			awtbprint(W_CMD, C_LABEL, ", cnt: ");
			awtbprint(W_CMD, C_DATA, "%i", b->value);
			awtbprint(W_CMD, C_LABEL, ")\n");
			b->value++;
			return b;
		}
		b = b->next;
	}
	return NULL;
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
void dbg_step()
{
	struct evlb_t *bhit = NULL;

	if ((!dbg_enter) && (!(bhit=dbg_brk_check()))) {
		return;
	}

	dbg_loop_fin = 0;

	while (!dbg_loop_fin) {
		if (aw_layout_changed) {
			awin_tb_scroll_end(W_CMD);
			aw_layout_redo();
		} else {
			aw_layout_refresh();
		}

		int res = aw_readline(W_CMD, C_PROMPT, "em400> ", C_INPUT, input_buf, INPUT_BUF_SIZE);

		if ((res == KEY_ENTER) && (*input_buf)) {
			if (ui_mode == O_NCURSES) {
				awtbprint(W_CMD, C_PROMPT, "%s", "em400> ");
				awtbprint(W_CMD, C_LABEL, "%s\n", input_buf);
				awin_tb_scroll_end(W_CMD);
			}
			int len = strlen(input_buf);
			input_buf[len] = '\n';
			input_buf[len+1] = '\0';
			dbg_parse(input_buf);
		} else if (res == KEY_NPAGE) {
			awin_tb_scroll(W_CMD, 10);
		} else if (res == KEY_PPAGE) {
			awin_tb_scroll(W_CMD, -10);
		}

	}
}


// vim: tabstop=4 shiftwidth=4 autoindent
