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

#include <string.h>
#include <stdlib.h>

#include "utils.h"
#include "errors.h"

#include "cpu/registers.h"

#include "debugger/dasm.h"
#include "debugger/awin.h"
#include "debugger/debugger.h"
#include "debugger/cmd.h"
#include "debugger/ui.h"


// -----------------------------------------------------------------------
int dbg_ui_init()
{
	// containers and windows are deleted during awin shutdown,
	// we need those pointers here only for access/checks

	AWIN *win;

	ACONT *c1 = aw_container_add(BOTTOM, LEFT, 1, 1, 0);
	if (c1 == NULL) return -1;
	win = aw_window_add(c1, W_STATUS, "Status", 0, 0, dbg_wu_status, FILL, 0, 0);
	if (win == NULL) return -1;

	ACONT *c2 = aw_container_add(TOP, LEFT, 20, 6, 30);
	if (c2 == NULL) return -1;
	win = aw_window_add(c2, W_STACK, "Stack", 1, 0, dbg_wu_stack, 17, 17, 50);
	if (win == NULL) return -1;
	win = aw_window_add(c2, W_MEM, "Memory", 1, 0, dbg_wu_mem, FILL, 30, 0);
	if (win == NULL) return -1;

	ACONT *c3 = aw_container_add(RIGHT, TOP, 30, 30, 30);
	if (c3 == NULL) return -1;
	win = aw_window_add(c3, W_DASM, "ASM", 1, 0, dbg_wu_dasm, FILL, 3, 0);
	if (win == NULL) return -1;

	ACONT *c4 = aw_container_add(TOP, LEFT, 11, 11, 15);
	if (c4 == NULL) return -1;
	win = aw_window_add(c4, W_SREGS, "System registers", 1, 0, dbg_wu_sregs, DIV2, 58, 58);
	if (win == NULL) return -1;
	win = aw_window_add(c4, W_REGS, "User registers", 1, 0, dbg_wu_regs, FILL, 58, 0);
	if (win == NULL) return -1;

	ACONT *c5 = aw_container_add(TOP, RIGHT, FILL, 0, 0);
	if (c5 == NULL) return -1;
	win = aw_window_add(c5, W_WATCH, "Watch", 1, 0, dbg_wu_watch, 30, 30, 30);
	if (win == NULL) return -1;
	win = aw_window_add(c5, W_CMD, "Commandline", 1, 1, dbg_wu_cmd, FILL, 0, 0);
	if (win == NULL) return -1;

	aw_attr_new(C_PROMPT, COLOR_BLACK, COLOR_YELLOW, A_BOLD);
	aw_attr_new(C_LABEL, COLOR_BLACK, COLOR_WHITE, A_NORMAL);
	aw_attr_new(C_DATA, COLOR_BLACK, COLOR_WHITE, A_BOLD);
	aw_attr_new(C_DATAU, COLOR_BLACK, COLOR_WHITE, A_BOLD|A_UNDERLINE);
	aw_attr_new(C_ILABEL, COLOR_WHITE, COLOR_BLACK, A_NORMAL);
	aw_attr_new(C_IDATA, COLOR_WHITE, COLOR_BLUE, A_NORMAL);
	aw_attr_new(C_ERROR, COLOR_BLACK, COLOR_RED, A_BOLD);
	aw_attr_new(C_READ, COLOR_BLACK, COLOR_GREEN, A_BOLD);
	aw_attr_new(C_WRITE, COLOR_RED, COLOR_WHITE, A_BOLD);
	aw_attr_new(C_RW, COLOR_RED, COLOR_GREEN, A_BOLD);

	return E_OK;
}

// -----------------------------------------------------------------------
void dbg_wu_mem(int wid)
{
	AWIN *w = aw_window_find(wid);
	if (!w) {
		return;
	}
	awin_tb_clear(wid);
	dbg_c_mem(wid, 0, 0, 0x300, w->iw, w->ih);
	awin_tb_update(wid, w->ih);
}

// -----------------------------------------------------------------------
void dbg_wu_dasm(int wid)
{
	AWIN *w = aw_window_find(wid);
	int offset = (w->ih) / 3;
	int start;
	if (regs[R_IC] < offset) {
		start = 0;
	} else {
		start = regs[R_IC] - offset;
	}
	awin_tb_clear(wid);
	dbg_c_dt(wid, DMODE_DASM, start, w->ih);
	awin_tb_update(wid, w->ih);
}

// -----------------------------------------------------------------------
void dbg_wu_regs(int wid)
{
	AWIN *w = aw_window_find(wid);
	awin_tb_clear(wid);
	dbg_c_regs(wid);
	awin_tb_update(wid, w->ih);
}

// -----------------------------------------------------------------------
void dbg_wu_sregs(int wid)
{
	AWIN *w = aw_window_find(wid);
	awin_tb_clear(wid);
	dbg_c_sregs(wid);
	awin_tb_update(wid, w->ih);
}

// -----------------------------------------------------------------------
void dbg_wu_cmd(int wid)
{
	AWIN *w = aw_window_find(wid);
	awin_tb_update(wid, w->ih-1);
}

// -----------------------------------------------------------------------
void dbg_wu_status(int wid)
{
	awfillbg(wid, C_ILABEL, ' ', 0);
}

// -----------------------------------------------------------------------
void dbg_wu_stack(int wid)
{
	AWIN *w = aw_window_find(wid);
	awin_tb_clear(wid);
	dbg_c_stack(wid, w->ih);
	awin_tb_update(wid, w->ih);
}

// -----------------------------------------------------------------------
void dbg_wu_watch(int wid)
{
	AWIN *w = aw_window_find(wid);
	awin_tb_clear(wid);
	dbg_c_watch_list(wid, w->ih);
	awin_tb_update(wid, w->ih);
}

// vim: tabstop=4 shiftwidth=4 autoindent
