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

#include <ctype.h>
#include <string.h>
#include <ncurses.h>
#include <stdio.h>
#include <signal.h>

#include "dasm.h"
#include "utils.h"
#include "errors.h"
#include "debuger.h"
#include "debuger_ui.h"

int nc_w_changed = 0;

struct e4d_w_struct e4d_w[] = {
	{ "Memory",				NULL, NULL, 0, 0, 0, 0, 1, 1, 0, em400_debuger_wu_mem },
	{ "ASM",				NULL, NULL, 0, 0, 0, 0, 1, 1, 0, em400_debuger_wu_dasm },
	{ "System registers",	NULL, NULL, 0, 0, 0, 0, 1, 1, 0, em400_debuger_wu_sregs },
	{ "User registers",		NULL, NULL, 0, 0, 0, 0, 1, 1, 0, em400_debuger_wu_regs },
	{ "Commandline",		NULL, NULL, 0, 0, 0, 0, 1, 1, 1, em400_debuger_wu_cmd },
	{ "Status",				NULL, NULL, 0, 0, 0, 0, 0, 1, 0, em400_debuger_wu_status },
	{ "",					NULL, NULL, 0, 0, 0, 0, 0, 0, 0, em400_debuger_wu_none }
};

int attr[C_MAX] = {0};

struct h_entry *history;
struct h_entry *cur_h;

// -----------------------------------------------------------------------
int em400_debuger_ui_init()
{
	// enter ncurses screen
	initscr();
	cbreak();
	noecho();
	start_color();

	// prepare history
	history = malloc(sizeof(struct h_entry));
	history->cmd = NULL;
	history->next = NULL;
	history->prev = NULL;
	history->len = 0;
	cur_h = history;

	// prepare handler for terminal resize
	struct sigaction sa;
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = _em400_debuger_w_resize_sig;

	if (sigemptyset(&sa.sa_mask) != 0) {
		return E_DEBUGER_SIG_RESIZE;
	}

	if (sigaction(SIGWINCH, &sa, NULL) != 0) {
		return E_DEBUGER_SIG_RESIZE;
	}

	// initialilze attributes

	// command line promt
	init_pair(C_PROMPT, COLOR_YELLOW, COLOR_BLACK);
	attr[C_PROMPT] = COLOR_PAIR(C_PROMPT) | A_BOLD;

	// status line label
	init_pair(C_ILABEL, COLOR_BLACK, COLOR_WHITE);
	attr[C_ILABEL] = COLOR_PAIR(C_ILABEL);

	// status line text
	//init_pair(C_IDATA, COLOR_BLUE, COLOR_WHITE);
	//attr[C_IDATA] = COLOR_PAIR(C_IDATA);
	attr[C_IDATA] = COLOR_PAIR(C_ILABEL) | A_BOLD;

	// standard label
	init_pair(C_LABEL, COLOR_WHITE, COLOR_BLACK);
	attr[C_LABEL] = COLOR_PAIR(C_LABEL);

	// standard data
	attr[C_DATA] = COLOR_PAIR(C_LABEL) | A_BOLD;

	// error
	init_pair(C_ERROR, COLOR_RED, COLOR_BLACK);
	attr[C_ERROR] = COLOR_PAIR(C_ERROR) | A_BOLD;

	return E_OK;
}

// -----------------------------------------------------------------------
void em400_debuger_ui_shutdown()
{
	endwin();
	nc_rl_histore_free(history);
}

// -----------------------------------------------------------------------
void nc_rl_history_add(char *cmd, int len)
{
	if ((history->prev) && (history->prev->cmd)) {
		int l = len;
		if (history->prev->len > len) {
			l = history->prev->len;
		}
		if (!strncmp(cmd, history->prev->cmd, l)) {
			return;
		}
	}

	struct h_entry *he = malloc(sizeof(struct h_entry));
	he->cmd = NULL;
	he->next = NULL;
	he->prev = history;
	he->len = 0;

	history->cmd = strdup(cmd);
	history->len = len;
	history->next = he;

	history = he;
}

// -----------------------------------------------------------------------
void nc_rl_histore_free(struct h_entry *h)
{
	if (!h) return;
	nc_rl_histore_free(h->prev);
	free(h->cmd);
	free(h);
}

// -----------------------------------------------------------------------
int nc_readline(WINDOW *win, const char *prompt, char *buffer, int buflen)
{
	int old_curs = curs_set(1);
	int pos = 0;
	int len = 0;
	int x, y;
	int c;

	flushinp();
	keypad(win, TRUE);
	getyx(win, y, x);
	mvwaprintw(win, y, 0, attr[C_PROMPT], prompt);
	getyx(win, y, x);

	while (1) {
		buffer[len] = ' ';
		mvwaddnstr(win, y, x, buffer, len+1);
		wmove(win, y, x+pos);
		c = wgetch(win);

		if ((c == KEY_ENTER) || (c == '\n') || (c == '\r')) {
			c = KEY_ENTER;
			if (len > 0) {
				nc_rl_history_add(buffer, len);
			}
			cur_h = history;
			buffer[len++] = '\n';
			wmove(win, y, x+len);
			break;
		} else if (isprint(c)) {
			if (pos < buflen-1) {
				memmove(buffer+pos+1, buffer+pos, len-pos);
				buffer[pos++] = c;
				len += 1;
			}
		} else if (c == KEY_LEFT) {
			if (pos > 0) {
				pos -= 1;
			}
		} else if (c == KEY_RIGHT) {
			if (pos < len) {
				pos += 1;
			}
		} else if (c == KEY_HOME) {
			pos = 0;
		} else if (c == KEY_END) {
			pos = len;
		} else if ((c == KEY_BACKSPACE) || (c == KEY_BACKSPACE2)) {
			if (pos > 0) {
				memmove(buffer+pos-1, buffer+pos, len-pos);
				pos -= 1;
				len -= 1;
			}
		} else if (c == KEY_DC) {
			if (pos < len) {
				memmove(buffer+pos, buffer+pos+1, len-pos-1);
				len -= 1;
			}
		} else if (c == KEY_UP) {
			if (cur_h->prev) {
				wmove(win, y, x);
				wprintw(win, "                                                                     ");
				cur_h = cur_h->prev;
				strcpy(buffer, cur_h->cmd);
				len = cur_h->len;
				pos = cur_h->len;
			}
		} else if (c == KEY_DOWN) {
			if (cur_h->next) {
				wmove(win, y, x);
				wprintw(win, "                                                                     ");
				cur_h = cur_h->next;
				if (cur_h->cmd) {
					strcpy(buffer, cur_h->cmd);
					len = cur_h->len;
					pos = cur_h->len;
				} else {
					len = 0;
					pos = 0;
				}
			}
		} else {
			break;
		}
	}

	buffer[len] = '\0';

	if (old_curs != ERR) {
		curs_set(old_curs);
	}
	return c;
}

// -----------------------------------------------------------------------
void e400_debuger_w_recalculate()
{
	// these are predefined
	e4d_w[WIN_STATUS].h = 1;
	e4d_w[WIN_DASM].w = 30;
	e4d_w[WIN_SREGS].h = 10;
	e4d_w[WIN_REGS].h = 10;

	// STATUS: always there, at the bottom
	e4d_w[WIN_STATUS].enabled = 1;
	e4d_w[WIN_STATUS].h = 1;
	e4d_w[WIN_STATUS].w = COLS;

	// DASM: width 28, full height
	e4d_w[WIN_DASM].enabled = 1;
	e4d_w[WIN_DASM].w = 30;
	e4d_w[WIN_DASM].h = LINES - e4d_w[WIN_STATUS].h;

	// MEM: at the top, fill horizontaly
	e4d_w[WIN_MEM].enabled = 1;
	e4d_w[WIN_MEM].w = COLS - e4d_w[WIN_DASM].w;
	e4d_w[WIN_MEM].h = (LINES - e4d_w[WIN_REGS].h - e4d_w[WIN_STATUS].h) / 2;
	// MEM: disable if <6
	if (e4d_w[WIN_MEM].h < 6) {
		e4d_w[WIN_MEM].enabled = 0;
		e4d_w[WIN_MEM].w = 0;
		e4d_w[WIN_MEM].h = 0;
	}
	if (e4d_w[WIN_MEM].h > 20 ) {
		e4d_w[WIN_MEM].h = 20;
	}

	// DASM: if terminal is too narrow, disable DASM
	if (COLS - e4d_w[WIN_DASM].w < 30) {
		e4d_w[WIN_DASM].enabled = 0;
		e4d_w[WIN_DASM].w = 0;
		e4d_w[WIN_DASM].w = 0;
	}

	// SREGS: height 10 sharp, width variable
	e4d_w[WIN_SREGS].enabled = 1;
	e4d_w[WIN_SREGS].h = 10;
	e4d_w[WIN_SREGS].w = (COLS - e4d_w[WIN_DASM].w) / 2;
	// SREGS: if width<58, disable
	if (e4d_w[WIN_SREGS].w < 58) {
		e4d_w[WIN_SREGS].enabled = 0;
		e4d_w[WIN_SREGS].w = 0;
		e4d_w[WIN_SREGS].h = 0;
	}

	// REGS: height 10 sharp, width variable
	e4d_w[WIN_REGS].enabled = 1;
	e4d_w[WIN_REGS].h = 10;
	e4d_w[WIN_REGS].w = COLS - e4d_w[WIN_DASM].w - e4d_w[WIN_SREGS].w;
	// REGS: if width<58, disable
	if ((e4d_w[WIN_REGS].w < 58) || (LINES - e4d_w[WIN_REGS].h - e4d_w[WIN_STATUS].h < 4)) {
		e4d_w[WIN_REGS].enabled = 0;
		e4d_w[WIN_REGS].w = 0;
		e4d_w[WIN_REGS].h = 0;
	}

	// CMD: 
	e4d_w[WIN_CMD].enabled = 1;
	e4d_w[WIN_CMD].w = COLS - e4d_w[WIN_DASM].w;
	e4d_w[WIN_CMD].h = LINES - e4d_w[WIN_REGS].h - e4d_w[WIN_STATUS].h - e4d_w[WIN_MEM].h;

	// POSITIONS

	e4d_w[WIN_STATUS].x = 0;
	e4d_w[WIN_STATUS].y = LINES-1;

	e4d_w[WIN_DASM].x = COLS - e4d_w[WIN_DASM].w;
	e4d_w[WIN_DASM].y = 0;

	e4d_w[WIN_REGS].x = 0;
	e4d_w[WIN_REGS].y = e4d_w[WIN_MEM].h;

	e4d_w[WIN_SREGS].x = e4d_w[WIN_REGS].w;
	e4d_w[WIN_SREGS].y = e4d_w[WIN_MEM].h;

	e4d_w[WIN_CMD].x = 0;
	e4d_w[WIN_CMD].y = e4d_w[WIN_MEM].h + e4d_w[WIN_REGS].h;

	e4d_w[WIN_MEM].x = 0;
	e4d_w[WIN_MEM].y = 0;
}

// -----------------------------------------------------------------------
void e400_debuger_w_destroy(int id)
{
	if ((!e4d_w[id].enabled) || (!e4d_w[id].win)) {
		return;
	}

	wrefresh(e4d_w[id].win);
	delwin(e4d_w[id].win);

	if (e4d_w[id].box) {
		wborder(e4d_w[id].bwin, ' ', ' ', ' ',' ',' ',' ',' ',' ');
		wrefresh(e4d_w[id].bwin);
		delwin(e4d_w[id].bwin);
	}
}

// -----------------------------------------------------------------------
void e400_debuger_w_create(int id)
{
	if (!e4d_w[id].enabled) {
		return;
	}

	if (e4d_w[id].box) {
		e4d_w[id].bwin = newwin(e4d_w[id].h, e4d_w[id].w, e4d_w[id].y, e4d_w[id].x);
		box(e4d_w[id].bwin, 0, 0);
		mvwaprintw(e4d_w[id].bwin, 0, 3, attr[C_LABEL], "[ %s ]", e4d_w[id].title);
		wrefresh(e4d_w[id].bwin);
		e4d_w[id].win = newwin(e4d_w[id].h-2, e4d_w[id].w-2, e4d_w[id].y+1, e4d_w[id].x+1);
	} else {
		e4d_w[id].win = newwin(e4d_w[id].h, e4d_w[id].w, e4d_w[id].y, e4d_w[id].x);
	}

	if (e4d_w[id].scrollable) {
		scrollok(e4d_w[id].win, TRUE);
	}

	e4d_w[id].fun(e4d_w[id].win);
	wrefresh(e4d_w[id].win);
}

// -----------------------------------------------------------------------
void e400_debuger_w_redraw_all()
{
	for (int i=WIN_MEM ; i<WIN_LAST ; i++) {
		if (e4d_w[i].enabled) {
			if (!e4d_w[i].scrollable) {
				wmove(e4d_w[i].win, 0, 0);
			}
			e4d_w[i].fun(e4d_w[i].win);
			wrefresh(e4d_w[i].win);
		}
	}
}

// -----------------------------------------------------------------------
void e400_debuger_w_reinit_all()
{
	int i;

	endwin();

	for (i=WIN_MEM ; i<WIN_LAST ; i++) {
		e400_debuger_w_destroy(i);
	}

	e400_debuger_w_recalculate();

	for (i=WIN_MEM ; i<WIN_LAST ; i++) {
		e400_debuger_w_create(i);
	}
}

// -----------------------------------------------------------------------
void _em400_debuger_w_resize_sig(int signum, siginfo_t *si, void *ctx)
{
	nc_w_changed = 1;
}

// -----------------------------------------------------------------------
void em400_debuger_wu_mem(WINDOW *win)
{
	em400_debuger_c_mem(e4d_w[WIN_MEM].win, 0, 0, 0xff);
}

// -----------------------------------------------------------------------
void em400_debuger_wu_dasm(WINDOW *win)
{
	int offset = (e4d_w[WIN_DASM].h-2) / 3;
	int start;
	if (R(R_IC) < offset) {
		start = 0;
	} else {
		start = R(R_IC) - offset;
	}
	em400_debuger_c_dt(win, DMODE_DASM, start, e4d_w[WIN_DASM].h-2);
}

// -----------------------------------------------------------------------
void em400_debuger_wu_regs(WINDOW *win)
{
	em400_debuger_c_regs(win);
}

// -----------------------------------------------------------------------
void em400_debuger_wu_sregs(WINDOW *win)
{
	em400_debuger_c_sregs(win);
}

// -----------------------------------------------------------------------
void em400_debuger_wu_cmd(WINDOW *win)
{
}

// -----------------------------------------------------------------------
void em400_debuger_wu_status(WINDOW *win)
{
	char *kb = int2bin(R(R_KB), 16);
	int x,y;
	wattron(win, attr[C_ILABEL]);
	whline(win, ' ', COLS);
	getyx(win, y, x);
	wmove(win, y, 0);
	waprintw(win, attr[C_ILABEL], "  Q:");
	waprintw(win, attr[C_IDATA], "%i", SR_Q);
	waprintw(win, attr[C_ILABEL], "  NB:");
	waprintw(win, attr[C_IDATA], "%i", SR_NB);
	waprintw(win, attr[C_ILABEL], "  IC:");
	waprintw(win, attr[C_IDATA], "0x%04x", R(R_IC));
	waprintw(win, attr[C_ILABEL], "  P:");
	waprintw(win, attr[C_IDATA], "%i", P);
	waprintw(win, attr[C_ILABEL], "  MOD:");
	waprintw(win, attr[C_IDATA], "%06i (0x%04x)", R(R_MOD), R(R_MOD));
	waprintw(win, attr[C_ILABEL], "  MODcnt:");
	waprintw(win, attr[C_IDATA], "%i", MODcnt);
	waprintw(win, attr[C_ILABEL], "  KB:");
	waprintw(win, attr[C_IDATA], "%s", kb);
	waprintw(win, attr[C_ILABEL], "  ZC17:");
	waprintw(win, attr[C_IDATA], "%i", ZC17);
	free(kb);
}

// -----------------------------------------------------------------------
void em400_debuger_wu_none(WINDOW *win)
{
}


// vim: tabstop=4
