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

#include <string.h>
#include <stdlib.h>

#include "dasm.h"
#include "utils.h"
#include "errors.h"
#include "awin.h"
#include "debugger.h"
#include "debugger_cmd.h"
#include "debugger_ui.h"


// -----------------------------------------------------------------------
void dbg_ui_init()
{
	ACONT *c1 = aw_container_add(BOTTOM, LEFT, 1, 1, 0);
	aw_window_add(c1, W_STATUS, "Status", 0, 0, dbg_wu_status, FILL, 0, 0);

	ACONT *c2 = aw_container_add(TOP, LEFT, 20, 6, 30);
	aw_window_add(c2, W_STACK, "Stack", 1, 0, dbg_wu_stack, 17, 17, 50);
	aw_window_add(c2, W_MEM, "Memory", 1, 0, dbg_wu_mem, FILL, 30, 0);

	ACONT *c3 = aw_container_add(RIGHT, TOP, 30, 30, 30);
	aw_window_add(c3, W_DASM, "ASM", 1, 0, dbg_wu_dasm, FILL, 3, 0);

	ACONT *c4 = aw_container_add(TOP, LEFT, 10, 10, 20);
	aw_window_add(c4, W_SREGS, "System registers", 1, 0, dbg_wu_sregs, DIV2, 58, 58);
	aw_window_add(c4, W_REGS, "User registers", 1, 0, dbg_wu_regs, FILL, 58, 0);

	ACONT *c5 = aw_container_add(TOP, LEFT, FILL, 0, 0);
	aw_window_add(c5, W_CMD, "Commandline", 1, 1, dbg_wu_cmd, FILL, 0, 0);

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
}

// -----------------------------------------------------------------------
void dbg_wu_mem(unsigned int wid)
{
	AWIN *w = aw_window_find(wid);
	if (!w) {
		return;
	}
	dbg_c_mem(wid, 0, 0, 0x300, w->iw, w->ih);
}

// -----------------------------------------------------------------------
void dbg_wu_dasm(unsigned int wid)
{
	AWIN *w = aw_window_find(wid);
	int offset = (w->ih) / 3;
	int start;
	if (regs[R_IC] < offset) {
		start = 0;
	} else {
		start = regs[R_IC] - offset;
	}
	dbg_c_dt(wid, DMODE_DASM, start, w->ih);
}

// -----------------------------------------------------------------------
void dbg_wu_regs(unsigned int wid)
{
	dbg_c_regs(wid);
}

// -----------------------------------------------------------------------
void dbg_wu_sregs(unsigned int wid)
{
	dbg_c_sregs(wid);
}

// -----------------------------------------------------------------------
void dbg_wu_cmd(unsigned int wid)
{
}

// -----------------------------------------------------------------------
void dbg_wu_status(unsigned int wid)
{
	char *kb = int2bin(regs[R_KB], 16);
	awfillbg(wid, C_ILABEL, ' ', 0);
	awprint(wid, C_ILABEL, " Q:");
	awprint(wid, C_IDATA, "%i", SR_Q);
	awprint(wid, C_ILABEL, "  NB:");
	awprint(wid, C_IDATA, "%i", SR_NB);
	awprint(wid, C_ILABEL, "  IC:");
	awprint(wid, C_IDATA, "0x%04x", regs[R_IC]);
	awprint(wid, C_ILABEL, "  P:");
	awprint(wid, C_IDATA, "%i", regs[R_P]);
	awprint(wid, C_ILABEL, "  MOD:");
	awprint(wid, C_IDATA, "%06i (0x%04x)", regs[R_MOD], regs[R_MOD]);
	awprint(wid, C_ILABEL, "  MODcnt:");
	awprint(wid, C_IDATA, "%i", regs[R_MODc]);
	awprint(wid, C_ILABEL, "  KB:");
	awprint(wid, C_IDATA, "%s", kb);
	awprint(wid, C_ILABEL, "  ZC17:");
	awprint(wid, C_IDATA, "%i", regs[R_ZC17]);
	free(kb);
}

// -----------------------------------------------------------------------
void dbg_wu_stack(unsigned int wid)
{
	AWIN *w = aw_window_find(wid);
	dbg_c_stack(wid, w->ih);
}

// vim: tabstop=4
