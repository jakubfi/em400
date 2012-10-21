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

#include "mjc400_dasm.h"
#include "em400_debuger.h"
#include "em400_debuger_ui.h"

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

// -----------------------------------------------------------------------
int nc_readline(WINDOW *win, const char *prompt, char *buffer, int buflen)
{
	int old_curs = curs_set(1);
	int pos = 0;
	int len = 0;
	int x, y;
	int c;

	keypad(win, TRUE);
	getyx(win, y, x);
	init_pair(1, COLOR_YELLOW, COLOR_BLACK);

	wattron(win, COLOR_PAIR(1));
	wattron(win, A_BOLD);
	mvwprintw(win, y, 0, prompt);
	wattroff(win, A_BOLD);
	wattroff(win, COLOR_PAIR(1));

	getyx(win, y, x);

	while (1) {
		buffer[len] = ' ';
		mvwaddnstr(win, y, x, buffer, len+1);
		wmove(win, y, x+pos);
		c = wgetch(win);

		if ((c == KEY_ENTER) || (c == '\n') || (c == '\r')) {
			wprintw(win, "\n");
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
		mvwprintw(e4d_w[id].bwin, 0, 3, "[ %s ]", e4d_w[id].title);
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
	__em400_debuger_dump_mem(e4d_w[WIN_MEM].win, 0, 0, 0x8f);
}

// -----------------------------------------------------------------------
void em400_debuger_wu_dasm(WINDOW *win)
{
	int offset = (e4d_w[WIN_DASM].h-2) / 3;
	int start;
	if (IC < offset) {
		start = 0;
	} else {
		start = IC - offset;
	}
	em400_debuger_dt(win, DMODE_DASM, start, e4d_w[WIN_DASM].h-2);
}

// -----------------------------------------------------------------------
void em400_debuger_wu_regs(WINDOW *win)
{
}

// -----------------------------------------------------------------------
void em400_debuger_wu_sregs(WINDOW *win)
{
}

// -----------------------------------------------------------------------
void em400_debuger_wu_cmd(WINDOW *win)
{
}

// -----------------------------------------------------------------------
void em400_debuger_wu_status(WINDOW *win)
{
	int x,y;
	init_pair(3, COLOR_BLUE, COLOR_WHITE);
	init_pair(4, COLOR_BLACK, COLOR_WHITE);
	wattron(win, COLOR_PAIR(3));
	whline(win, ' ', COLS);
	getyx(win, y, x);
	wmove(win, y, 0);
	wattron(win, COLOR_PAIR(4)); wprintw(win, "  Q:");
	wattron(win, COLOR_PAIR(3)); wprintw(win, "%i", SR_Q);
	wattron(win, COLOR_PAIR(4)); wprintw(win, "  NB:");
	wattron(win, COLOR_PAIR(3)); wprintw(win, "%i", SR_NB);
	wattron(win, COLOR_PAIR(4)); wprintw(win, "  IC:");
	wattron(win, COLOR_PAIR(3)); wprintw(win, "0x%04x", IC);
	wattron(win, COLOR_PAIR(4)); wprintw(win, "  P:");
	wattron(win, COLOR_PAIR(3)); wprintw(win, "%i", P);
	wattron(win, COLOR_PAIR(4)); wprintw(win, "  MOD:");
	wattron(win, COLOR_PAIR(3)); wprintw(win, "%06i (0x%04x)", MOD, MOD);
	wattron(win, COLOR_PAIR(4)); wprintw(win, "  MODcnt:");
	wattron(win, COLOR_PAIR(3)); wprintw(win, "%i", MODcnt);
	wattron(win, COLOR_PAIR(4)); wprintw(win, "  ZC17:");
	wattron(win, COLOR_PAIR(3)); wprintw(win, "%i", ZC17);
	wattroff(win, COLOR_PAIR(3));
}

// -----------------------------------------------------------------------
void em400_debuger_wu_none(WINDOW *win)
{
}


// vim: tabstop=4
