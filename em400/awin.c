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

#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "awin.h"

volatile int aw_layout_changed;

int aw_attr[64];
int aw_output;

struct h_entry *aw_history, *aw_history_cur;

ACONT *aw_containers, *aw_container_last;

int wmin, wmax;
int chmin, chmax, cvmin, cvmax;

// -----------------------------------------------------------------------
void _aw_resize_sig(int signum, siginfo_t *si, void *ctx)
{
	aw_layout_changed = 1;
}

// -----------------------------------------------------------------------
int aw_init(int output)
{
	aw_output = output;

	NCCHECK 0;

	initscr();
	cbreak();
	noecho();
	start_color();

	// prepare handler for terminal resize
	struct sigaction sa;
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = _aw_resize_sig;

	if (sigemptyset(&sa.sa_mask) != 0) {
		return -1;
	}

	if (sigaction(SIGWINCH, &sa, NULL) != 0) {
		return -1;
	}

	return 0;
}

// -----------------------------------------------------------------------
void aw_shutdown()
{
	if (aw_output != O_NCURSES) {
		return;
	}
	aw_nc_rl_history_delete(aw_history);
	aw_layout_remove();
	aw_containers_all_delete(aw_containers);
	endwin();
}

// -----------------------------------------------------------------------
int aw_fit(int l_av, int l_max, int l_min, int l_left)
{
	// there is no space for l_min+l_left nothing to do
	if (l_av < l_min + l_left) {
		return 0;
	}

	int len;

	// translate "special" requirements to actual lengths
	switch (l_max) {
		case FILL:
			len = l_av - l_left;
			break;
		case DIV2:
			len = l_av/2;
			break;
		default:
			len = l_max;
	}

	// shrink up to l_min 
	while ((len < l_min) || (l_av - len < l_left)) {
		len--;
	}
	return len;
}

// -----------------------------------------------------------------------
void aw_windows_all_delete(AWIN *w)
{
	if (!w) {
		return;
	}
	aw_windows_all_delete(w->next);
	free(w->title);
	aw_window_nc_delete(w);
}

// -----------------------------------------------------------------------
void aw_containers_all_delete(ACONT *c)
{
	if (!c) {
		return;
	}
	aw_containers_all_delete(c->next);
	aw_windows_all_delete(c->win);
}

// -----------------------------------------------------------------------
AWIN * aw_window_find(unsigned int id)
{
	NCCHECK NULL;
	ACONT *c = aw_containers;
	while (c) {
		AWIN *w = c->win;
		while (w) {
			if (w->id == id) {
				return w;
			}
			w = w->next;
		}
		c = c->next;
	}
	return NULL;
}

// -----------------------------------------------------------------------
void aw_window_disable(AWIN *w)
{
	w->enabled = 0;
	w->w = 0;
	w->h = 0;
	w->x = 0;
	w->y = 0;
}

// -----------------------------------------------------------------------
void aw_container_disable(ACONT *c)
{
	// disable all windows in this container
	AWIN *w = c->win;
	while (w) {
		aw_window_disable(w);
		w = w->next;
	}

	c->x = 0;
	c->y = 0;

	// if container size is >0, we need to re-adjust container boundaries
	if (c->size > 0) {
		switch (c->calign) {
			case TOP:
				cvmin -= c->size;
				break;
			case BOTTOM:
				cvmax += c->size;
				break;
			case LEFT:
				chmin -= c->size;
				break;
			case RIGHT:
				chmax += c->size;
				break;
		}
	}

	c->size = 0;
}

// -----------------------------------------------------------------------
int aw_container_fit(ACONT *c)
{
	int avail;

	// get available space for container
	if (c->calign & HORIZONTAL) {
		avail = cvmax - cvmin;
	} else {
		avail = chmax - chmin;
	}

	// calculate container size
	c->size = aw_fit(avail, c->max, c->min, c->left);

	// disable it, if doesn't fit;
	if (c->size <= 0) {
		return 0;
	}

	// update window bountaries for the container
	if (c->calign & HORIZONTAL) {
		wmin = chmin;
		wmax = chmax;
	} else {
		wmin = cvmin;
		wmax = cvmax;
	}

	// update container boundaries, set container's (x,y)
	switch (c->calign) {
		case TOP:
			c->x = chmin;
			c->y = cvmin;
			cvmin += c->size;
			break;
		case BOTTOM:
			c->x = chmin;
			c->y = cvmax - c->size;
			cvmax -= c->size;
			break;
		case LEFT:
			c->x = chmin;
			c->y = cvmin;
			chmin += c->size;
			break;
		case RIGHT:
			c->x = chmax - c->size;
			c->y = cvmin;
			chmax -= c->size;
			break;
	}

	return c->size;
}

// -----------------------------------------------------------------------
ACONT * aw_container_add(int calign, int walign, int max, int min, int left)
{
	NCCHECK NULL;
	ACONT *c = malloc(sizeof(ACONT));
	if (!c) {
		return NULL;
	}

	c->calign = calign;
	c->walign = walign;
	c->min = min;
	c->max = max;
	c->left = left;
	c->size = 0;
	c->x = 0;
	c->y = 0;
	c->next = NULL;
	c->win = NULL;
	c->win_last = NULL;

	// add container to the list
	if (!aw_containers) {
		aw_containers = aw_container_last = c;
	} else {
		aw_container_last->next = c;
		aw_container_last = c;
	}

	return c;
}

// -----------------------------------------------------------------------
int aw_window_fit(ACONT *c, AWIN *w)
{
	// calculate window size
	int size = aw_fit(wmax - wmin, w->max, w->min, w->left);

	// disable it, if doesn't fit
	if (size <= 0) {
		w->enabled = 0;
		return 0;
	}

	w->enabled = 1;

	// update window's width/height
	if (c->calign & HORIZONTAL) {
		w->w = size;
		w->h = c->size;
		if (c->walign == LEFT) {
			w->x = wmin;
			w->y = c->y;
		} else {
			w->x = wmax - w->w;
			w->y = c->y;
		}
	} else {
		w->w = c->size;
		w->h = size;
		if (c->walign == TOP) {
			w->x = c->x;
			w->y = wmin;
		} else {
			w->x = c->x;
			w->y = wmax - w->h;
		}
	}

	// set new window boundaries
	if ((c->walign == LEFT) || (c->walign == TOP)) {
		wmin += size;
	} else {
		wmax -= size;
	}

	return size;
}

// -----------------------------------------------------------------------
AWIN * aw_window_add(ACONT *container, unsigned int id, char *title, int border, int scrollable, void (*fun)(unsigned int wid), int max, int min, int left)
{
	NCCHECK NULL;
	if (!container) {
		return NULL;
	}

	if (aw_window_find(id)) {
		return NULL;
	}

	AWIN *w = malloc(sizeof(AWIN));
	if (!w) {
		return NULL;
	}

	w->id = id;
	w->title = strdup(title);
	w->win = NULL;
	w->bwin = NULL;
	w->w = 0;
	w->h = 0;
	w->x = 0;
	w->y = 0;
	w->max = max;
	w->min = min;
	w->left = left;
	w->border = border;
	w->enabled = 0;
	w->scrollable = scrollable;
	w->fun = fun;
	w->next = NULL;

	// add window to the list
	if (container->win_last) {
		container->win_last->next = w;
		container->win_last = w;
	} else {
		container->win_last = container->win = w;
	}

	return w;
}

// -----------------------------------------------------------------------
void aw_window_nc_delete(AWIN *w)
{
	if (!w) {
		return;
	}
	wborder(w->bwin, ' ', ' ', ' ',' ',' ',' ',' ',' ');
	if (w->win) {
		wrefresh(w->win);
		delwin(w->win);
		w->win = NULL;
	}
	if (w->border && w->bwin) {
		wrefresh(w->bwin);
		delwin(w->bwin);
		w->bwin = NULL;
	}
}

// -----------------------------------------------------------------------
void aw_window_nc_create(AWIN *w)
{
	if (!w) {
		return;
	}
	if (w->border) {
		w->bwin = newwin(w->h, w->w, w->y, w->x);
		w->win = newwin(w->h-2, w->w-2, w->y+1, w->x+1);
		box(w->bwin, 0, 0);
		mvwprintw(w->bwin, 0, 3, "[ %s ]", w->title);
	} else {
		w->win = newwin(w->h, w->w, w->y, w->x);
	}

	if (w->scrollable) {
		scrollok(w->win, TRUE);
	}
}

// -----------------------------------------------------------------------
void aw_layout_calculate()
{
	ACONT *c = aw_containers;
	int csize, twsize;
	AWIN *w;

	while (c) {
		// fit container
		csize = aw_container_fit(c);

		twsize = 0;
		w = c->win;

		while (w) {
			// if container is disabled, disable all windows
			if (csize <= 0) {
				aw_window_disable(w);
			// if container fits, fit each windows
			} else {
				twsize += aw_window_fit(c, w);
			}
			w = w->next;
		}

		// if none of the windows fit, disable container too
		if (twsize <= 0) {
			aw_container_disable(c);
		}
		c = c->next;
	}
}

// -----------------------------------------------------------------------
void aw_layout_create()
{
	ACONT *c = aw_containers;
	while (c) {
		if (c->size > 0) {
			AWIN *w = c->win;
			while (w) {
				if (w->enabled) {
					aw_window_nc_create(w);
				}
				w = w->next;
			}
		}
		c = c->next;
	}
}

// -----------------------------------------------------------------------
void aw_layout_remove()
{
	ACONT *c = aw_containers;
	while (c) {
		AWIN *w = c->win;
		while (w) {
			aw_window_nc_delete(w);
			w = w->next;
		}
		c = c->next;
	}
}

// -----------------------------------------------------------------------
void aw_layout_refresh()
{
	NCCHECK;
	ACONT *c = aw_containers;
	while (c) {
		AWIN *w = c->win;
		while (w) {
			if (!w->scrollable) {
				wmove(w->win, 0, 0);
			}
			w->fun(w->id);
			wrefresh(w->bwin);
			wrefresh(w->win);
			w = w->next;
		}
		c = c->next;
	}
}

// -----------------------------------------------------------------------
void aw_layout_redo()
{
	NCCHECK;
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	resizeterm(w.ws_row, w.ws_col);

	chmin = 0;
	chmax = COLS;
	cvmin = 0;
	cvmax = LINES;

	aw_layout_remove();
	aw_layout_calculate();
	aw_layout_create();
	aw_layout_refresh();

	aw_layout_changed = 0;
}

// -----------------------------------------------------------------------
void aw_attr_new(int id, int bgcolor, int fgcolor, int attr)
{
	NCCHECK;
	init_pair(id, fgcolor, bgcolor);
	aw_attr[id] = COLOR_PAIR(id) | attr;
}

// -----------------------------------------------------------------------
void awxyprint(int id, int x, int y, int attr, char *format, ...)
{
	AWIN *w = aw_window_find(id);
	if ((aw_output == O_NCURSES) && (!w)) {
		return;
	}

	va_list vl;
	va_start(vl, format);

	switch (aw_output) {
		case O_NCURSES:
			wattron(w->win, aw_attr[attr]);
			if (x>=0 && y>=0) {
				wmove(w->win, y, x);
			}
			vwprintw(w->win, format, vl);
			wattroff(w->win, aw_attr[attr]);
			break;
		case O_STD:
			vprintf(format, vl);
			break;
	}

	va_end(vl);
}

// -----------------------------------------------------------------------
void awfillbg(int id, int attr, char c, int len)
{
	AWIN *w = aw_window_find(id);
	if ((aw_output == O_NCURSES) && (!w)) {
		return;
	}

	int x, y;
	getyx(w->win, y, x);

	if (len <= 0) {
		len = w->w - x - 3;
	}
	char *fill = malloc(len+1);
	memset(fill, c, len);
	fill[len] = '\0';

	wattron(w->win, aw_attr[attr]);
	wprintw(w->win, fill);
	wmove(w->win, y, x);
	wattroff(w->win, aw_attr[attr]);

	free(fill);
}

// -----------------------------------------------------------------------
void aw_nc_rl_history_add(char *cmd, int len)
{
	aw_history_cur = NULL;

	if (aw_history) {
		int l = len;
		if (aw_history->len > len) {
			l = aw_history->len;
		}
		if (!strncmp(cmd, aw_history->cmd, l)) {
			return;
		}
	}

	struct h_entry *he = malloc(sizeof(struct h_entry));
	he->cmd = strdup(cmd);
	he->len = len;
	he->next = NULL;
	he->prev = aw_history;

	if (aw_history) {
		aw_history->next = he;
	}
	aw_history = he;
}

// -----------------------------------------------------------------------
void aw_nc_rl_history_delete(struct h_entry *h)
{
	if (!h) return;
	aw_nc_rl_history_delete(h->prev);
	free(h->cmd);
	free(h);
}

// -----------------------------------------------------------------------
struct h_entry * aw_nc_rl_history_get_prev()
{
	if (!aw_history_cur) {
		aw_history_cur = aw_history;
		return aw_history;
	}

	if (aw_history_cur->prev) {
		aw_history_cur = aw_history_cur->prev;
	}

	return aw_history_cur;
}

// -----------------------------------------------------------------------
struct h_entry * aw_nc_rl_history_get_next()
{
	if (!aw_history_cur) {
		return NULL;
	}

	aw_history_cur = aw_history_cur->next;
	return aw_history_cur;
}

// -----------------------------------------------------------------------
int aw_nc_readline(int id, int attr, char *prompt, char *buffer, int buflen)
{
	int pos = 0;
	int len = 0;
	int x, y;
	int c;
	struct h_entry *he;

	AWIN *w = aw_window_find(id);
	if (!w) {
		return -1;
	}

	flushinp();
	keypad(w->win, TRUE);
	getyx(w->win, y, x);
	wattron(w->win, aw_attr[attr]);
	mvwprintw(w->win, y, 0, prompt);
	wattroff(w->win, aw_attr[attr]);
	getyx(w->win, y, x);

	while (1) {
		buffer[len] = ' ';
		mvwaddnstr(w->win, y, x, buffer, len+1);
		wmove(w->win, y, x+pos);
		c = wgetch(w->win);

		if ((c == KEY_ENTER) || (c == '\n') || (c == '\r')) {
			c = KEY_ENTER;
			if (len > 0) {
				aw_nc_rl_history_add(buffer, len);
			}
			buffer[len++] = '\n';
			wmove(w->win, y, x+len);
			wprintw(w->win, "\n");
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
			he = aw_nc_rl_history_get_prev();
			if (he) {
				wmove(w->win, y, x);
				awfillbg(id, 0, ' ', -1);
				strcpy(buffer, he->cmd);
				len = he->len;
				pos = he->len;
			}
		} else if (c == KEY_DOWN) {
			wmove(w->win, y, x);
			awfillbg(id, 0, ' ', -1);
			he = aw_nc_rl_history_get_next();
			if (he) {
				strcpy(buffer, he->cmd);
				len = he->len;
				pos = he->len;
			} else {
				len = 0;
				pos = 0;
			}
		} else {
			break;
		}
	}
	buffer[len] = '\0';

	return c;
}

// -----------------------------------------------------------------------
int aw_readline(int id, int attr, char *prompt, char *buffer, int buflen)
{
	char *rlin;
	switch (aw_output) {
		case O_NCURSES:
			return aw_nc_readline(id, attr, prompt, buffer, buflen);
		case O_STD:
			rl_bind_key('\t', rl_abort);
			rlin = readline(prompt);
			if ((rlin) && (*rlin)) {
				add_history(rlin);
				strncpy(buffer, rlin, strlen(rlin));
				buffer[strlen(rlin)] = '\n';
				buffer[strlen(rlin)+1] = '\0';
				free(rlin);
				return KEY_ENTER;
			} else {
				return -1;
			}
			break;
	}
	return -1;
}

// vim: tabstop=4
