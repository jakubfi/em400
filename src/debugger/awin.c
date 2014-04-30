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

#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ncurses.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "utils.h"
#include "debugger/awin.h"

volatile int aw_layout_changed;

int aw_attr[64];
int aw_output;
char *aw_hist_file;

struct h_entry *aw_history, *aw_history_cur;

ACONT *aw_containers, *aw_container_last;

int wmin, wmax;
int chmin, chmax, cvmin, cvmax;

// -----------------------------------------------------------------------
static void _aw_sigwinch_handler(int signum)
{
	fprintf(stderr, "Window resized\n");
	aw_layout_changed = 1;
}

// -----------------------------------------------------------------------
void aw_rl_history_read(char *hist_file)
{
	char buf[1024];
	char *b = buf;
	FILE *hf = fopen(hist_file, "r");
	if (!hf) {
		return;
	}
	while (1) {
		int c = fread(b, 1, 1, hf);
		if (c <= 0) {
			break;
		}
		if (*b == '\n') {
			aw_nc_rl_history_add(buf, b-buf);
			b = buf;
		} else {
			b++;
		}
	}
	fclose(hf);
}

// -----------------------------------------------------------------------
void aw_rl_history_write(char *hist_file)
{
	FILE *hf = fopen(hist_file, "w");
	if (!hf) {
		return;
	}

	struct h_entry *h = aw_history;
	while (h && h->prev) {
		h = h->prev;
	}

	while (h) {
		int res = 0;
		res += fwrite(h->cmd, h->len, 1, hf);
		res = fwrite("\n", 1, 1, hf);
		h = h->next;
	}

	fclose(hf);
}

// -----------------------------------------------------------------------
void aw_read_history()
{
	if (aw_hist_file) {
		if (aw_output == O_STD) {
			read_history(aw_hist_file);
		} else if (aw_output == O_NCURSES) {
			aw_rl_history_read(aw_hist_file);
		}
	}
}

// -----------------------------------------------------------------------
void aw_write_history()
{
	if (aw_hist_file) {
		if (aw_output == O_STD) {
			write_history(aw_hist_file);
		} else if (aw_output == O_NCURSES) {
			aw_rl_history_write(aw_hist_file);
		}
	}
}

// -----------------------------------------------------------------------
int aw_init(int output, char *history)
{
	aw_output = output;
	aw_hist_file = history;

	aw_read_history();

	NCCHECK 0;

	initscr();
	cbreak();
	noecho();
	start_color();

	if (signal(SIGWINCH, _aw_sigwinch_handler) == SIG_ERR) {
		return -1;
	}

	return 0;
}

// -----------------------------------------------------------------------
void aw_shutdown()
{
	aw_write_history();
	if (aw_output != O_NCURSES) {
		clear_history();
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
AWIN * aw_window_find(int id)
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

	w->bw = 0;
	w->bh = 0;
	w->bx = 0;
	w->by = 0;

	w->iw = 0;
	w->ih = 0;
	w->ix = 0;
	w->iy = 0;
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
		w->bw = size;
		w->bh = c->size;
		if (c->walign == LEFT) {
			w->bx = wmin;
			w->by = c->y;
		} else {
			w->bx = wmax - w->bw;
			w->by = c->y;
		}
	} else {
		w->bw = c->size;
		w->bh = size;
		if (c->walign == TOP) {
			w->bx = c->x;
			w->by = wmin;
		} else {
			w->bx = c->x;
			w->by = wmax - w->bh;
		}
	}

	// set new window boundaries
	if ((c->walign == LEFT) || (c->walign == TOP)) {
		wmin += size;
	} else {
		wmax -= size;
	}

	w->ix = w->bx;
	w->iy = w->by;
	w->iw = w->bw;
	w->ih = w->bh;

	if (w->border) {
		w->ix += 1;
		w->iy += 1;
		w->iw -= 2;
		w->ih -= 2;
	}

	return size;
}

// -----------------------------------------------------------------------
AWIN * aw_window_add(ACONT *container, int id, char *title, int border, int scrollable, void (*fun)(int wid), int max, int min, int left, int attr)
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
	w->max = max;
	w->min = min;
	w->left = left;
	w->border = border;
	w->battr = attr;
	w->scrollable = scrollable;
	w->fun = fun;
	w->tb = calloc(1, sizeof(struct awin_tb));
	w->tb->maxlines = 1024;
	w->next = NULL;

	// reset x/y/w/h
	aw_window_disable(w);

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
		w->bwin = newwin(w->bh, w->bw, w->by, w->bx);
		wattron(w->bwin, aw_attr[w->battr]);
		box(w->bwin, 0, 0);
		mvwprintw(w->bwin, 0, 3, "[ %s ]", w->title);
		wattroff(w->bwin, aw_attr[w->battr]);
	}
	w->win = newwin(w->ih, w->iw, w->iy, w->ix);

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
void aw_clear_win(int id)
{
	NCCHECK;
	AWIN *w = aw_window_find(id);
	werase(w->win);
}

// -----------------------------------------------------------------------
void awprint(int id, int attr, char *format, ...)
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
void awtbbinprint(int id, int attr, char *format, uint32_t value, int size)
{
	char *buf = int2binf(format, value, size);
	awtbprint(id, attr, "%s", buf);
	free(buf);
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

	if (len == 0) {
		len = w->iw - x;
	} else if (len < 0) {
		len = w->iw - x + len;
		if (len < 0) {
			len = 0;
		}
	}

	char *fill = malloc(len+1);
	if (len > 0) {
		memset(fill, c, len);
	}
	fill[len] = '\0';

	wattron(w->win, aw_attr[attr]);
	wprintw(w->win, "%s", fill);
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
int aw_nc_readline(int id, int pattr, char *prompt, int iattr, char *buffer, int buflen)
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
	wattron(w->win, aw_attr[pattr]);
	mvwprintw(w->win, y, 0, "%s", prompt);
	wattroff(w->win, aw_attr[pattr]);
	getyx(w->win, y, x);

	wattron(w->win, aw_attr[iattr]);
	while (1) {
		buffer[len] = ' ';
		mvwaddnstr(w->win, y, x, buffer, len+1);
		wmove(w->win, y, x+pos);
		c = wgetch(w->win);

		if ((c == KEY_ENTER) || (c == '\n') || (c == '\r')) {
			c = KEY_ENTER;
			if ((len > 0) && (strncasecmp(buffer, "quit", 4))) {
				aw_nc_rl_history_add(buffer, len);
			}
			wmove(w->win, y, x+len);
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
	wattroff(w->win, aw_attr[iattr]);
	buffer[len] = '\0';

	return c;
}

// -----------------------------------------------------------------------
int aw_readline(int id, int pattr, char *prompt, int iattr, char *buffer, int buflen)
{
	char *rlin;
	switch (aw_output) {
		case O_NCURSES:
			return aw_nc_readline(id, pattr, prompt, iattr, buffer, buflen);
		case O_STD:
			rl_bind_key('\t', rl_abort);
			rlin = readline(prompt);
			if ((rlin) && (*rlin)) {
				if (strncasecmp(rlin, "quit", 4)) {
					add_history(rlin);
				}
				strcpy(buffer, rlin);
				free(rlin);
				return KEY_ENTER;
			} else {
				free(rlin);
				return -1;
			}
			break;
	}
	return -1;
}

// -----------------------------------------------------------------------
void awin_tb_append(struct awin_tb *tb, struct awin_tb_line *line)
{
	if (!tb) {
		return;
	}
	if (tb->end) {
		tb->end->next = line;
		line->prev = tb->end;
		tb->end = line;
	} else {
		tb->end = tb->beg = line;
		tb->disp_beg = NULL;
	}
	line->num = tb->lines;
	tb->lines++;
}

// -----------------------------------------------------------------------
void awin_tb_line_append(struct awin_tb_line *line, struct awin_tb_fragment *fragment)
{
	if (!line) {
		return;
	}
	if (line->end) {
		line->end->next = fragment;
		line->end = fragment;
	} else {
		line->end = line->beg = fragment;
	}
	line->len += fragment->len;
}


// -----------------------------------------------------------------------
struct awin_tb_line * aw_tb_get_last(struct awin_tb *tb, int lines)
{
	struct awin_tb_line *line = tb->end;
	while (line && line->prev && (lines > 0)) {
		line = line->prev;
		lines--;
	}
	return line;
}

// -----------------------------------------------------------------------
void awin_tb_scroll(int wid, int lines)
{
	NCCHECK;
	AWIN *win = aw_window_find(wid);

	struct awin_tb_line *line;
	// scroll up
	if (lines < 0) {
		if (win->tb->disp_beg) {
			line = win->tb->disp_beg;
		} else {
			line = aw_tb_get_last(win->tb, win->ih-1);
		}
		while (line && line->prev && (lines < 0)) {
			line = line->prev;
			lines++;
		}
	// scroll down
	} else {
		if (win->tb->disp_beg) {
			line = win->tb->disp_beg;
		} else {
			line = win->tb->beg;
		}
		while (line && line->next && (lines > 0) && (win->tb->end->num - line->num > (win->ih-1))) {
			line = line->next;
			lines--;
		}
	}
	win->tb->disp_beg = line;
}

// -----------------------------------------------------------------------
void awin_tb_scroll_end(int wid)
{
	NCCHECK;
	AWIN *win = aw_window_find(wid);
	win->tb->disp_beg = NULL;
}

// -----------------------------------------------------------------------
void awin_tb_scroll_home(int wid)
{
	NCCHECK;
	AWIN *win = aw_window_find(wid);
	win->tb->disp_beg = win->tb->beg;
}

// -----------------------------------------------------------------------
void awtbprint(int wid, int attr, char *format, ...)
{
	char buf[32000];

	va_list vl;
	va_start(vl, format);
	int len = vsprintf(buf, format, vl);
	va_end(vl);

	if (aw_output == O_NCURSES) {
		AWIN *win = aw_window_find(wid);
		char *beg = buf;
		char *n;
		int flen;



		while (beg && *beg) {
			n = strchr(beg, '\n');
			if (n) {
				flen = n - beg + 1;
			} else {
				flen = len;
			}

			struct awin_tb_fragment *fragment = calloc(1, sizeof(struct awin_tb_fragment));
			fragment->len = flen;
			fragment->text = calloc(1, flen+1);
			memcpy(fragment->text, beg, flen);
			fragment->attr = attr;

			// if tb is empty, create an empty line
			if (!win->tb->end) {
				awin_tb_append(win->tb, calloc(1, sizeof(struct awin_tb_line)));
			}
			// append fragment
			awin_tb_line_append(win->tb->end, fragment);
			// if nl, create new empty line
			if (n) {
				awin_tb_append(win->tb, calloc(1, sizeof(struct awin_tb_line)));
				beg = n+1;
			} else {
				beg = NULL;
			}
		}


	} else if (aw_output == O_STD) {
		printf("%s", buf);
	}
}

// -----------------------------------------------------------------------
void awin_tb_update(int wid, int height)
{
	AWIN *win = aw_window_find(wid);

	struct awin_tb_line *line;
	if (!win->tb->disp_beg) {
		line = aw_tb_get_last(win->tb, height);
	} else {
		line = win->tb->disp_beg;
	}

	aw_clear_win(wid);
	int cur_line = 0;
	while (line && (cur_line < height)) {
		struct awin_tb_fragment *fragment = line->beg;
		while (fragment) {
			awprint(wid, fragment->attr, "%s", fragment->text);
			fragment = fragment->next;
		}
		line = line->next;
		cur_line++;
	}
}

// -----------------------------------------------------------------------
void awin_tb_drop_fragments(struct awin_tb_fragment *fragment)
{
	struct awin_tb_fragment *f = fragment;
	while (f) {
		struct awin_tb_fragment *next = f->next;
		free(f->text);
		free(f);
		f = next;
	}
}

// -----------------------------------------------------------------------
void awin_tb_drop_lines(struct awin_tb_line *line)
{
	struct awin_tb_line *l = line;
	while (l) {
		struct awin_tb_line *next = l->next;
		awin_tb_drop_fragments(l->beg);
		free(l);
		l = next;
	}
}

// -----------------------------------------------------------------------
void awin_tb_clear(int wid)
{
	AWIN *win = aw_window_find(wid);
	awin_tb_drop_lines(win->tb->beg);
	win->tb->beg = win->tb->end = win->tb->disp_beg = NULL;
	win->tb->lines = 0;
	win->tb->maxlines = 1024;
}


// vim: tabstop=4 shiftwidth=4 autoindent
