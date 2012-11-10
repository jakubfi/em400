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
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "dasm.h"
#include "utils.h"
#include "errors.h"
#include "awin.h"
#include "debuger.h"
#include "debuger_cmd.h"
#include "debuger_ui.h"


// -----------------------------------------------------------------------
void em400_debuger_ui_init()
{
    ACONT *c1 = aw_container_add(BOTTOM, LEFT, 1, 1, 0);
    aw_window_add(c1, W_STATUS, "Status", 0, 0, em400_debuger_wu_status, FILL, 0, 0);

    ACONT *c2 = aw_container_add(RIGHT, TOP, 30, 30, 30);
    aw_window_add(c2, W_DASM, "ASM", 1, 0, em400_debuger_wu_dasm, FILL, 3, 0);

    ACONT *c3 = aw_container_add(TOP, LEFT, 20, 6, 30);
    aw_window_add(c3, W_MEM, "Memory", 1, 0, em400_debuger_wu_mem, FILL, 30, 0);

    ACONT *c4 = aw_container_add(TOP, LEFT, 10, 10, 20);
    aw_window_add(c4, W_SREGS, "System registers", 1, 0, em400_debuger_wu_sregs, DIV2, 58, 58);
    aw_window_add(c4, W_REGS, "User registers", 1, 0, em400_debuger_wu_regs, FILL, 58, 0);

    ACONT *c5 = aw_container_add(TOP, LEFT, FILL, 0, 0);
    aw_window_add(c5, W_CMD, "Commandline", 1, 1, em400_debuger_wu_cmd, FILL, 0, 0);

	aw_attr_new(C_PROMPT, COLOR_BLACK, COLOR_YELLOW, A_BOLD);
	aw_attr_new(C_LABEL, COLOR_BLACK, COLOR_WHITE, 0);
	aw_attr_new(C_DATA, COLOR_BLACK, COLOR_WHITE, A_BOLD);
	aw_attr_new(C_ILABEL, COLOR_WHITE, COLOR_BLACK, 0);
	aw_attr_new(C_IDATA, COLOR_WHITE, COLOR_BLUE, 0);
	aw_attr_new(C_ERROR, COLOR_BLACK, COLOR_RED, A_BOLD);
}

// -----------------------------------------------------------------------
void em400_debuger_wu_mem(unsigned int wid)
{
	em400_debuger_c_mem(wid, 0, 0, 0xff);
}

// -----------------------------------------------------------------------
void em400_debuger_wu_dasm(unsigned int wid)
{
	AWIN *w = aw_window_find(wid);
	int offset = (w->h - 2) / 3;
	int start;
	if (R(R_IC) < offset) {
		start = 0;
	} else {
		start = R(R_IC) - offset;
	}
	em400_debuger_c_dt(wid, DMODE_DASM, start, w->h - 2);
}

// -----------------------------------------------------------------------
void em400_debuger_wu_regs(unsigned int wid)
{
	em400_debuger_c_regs(wid);
}

// -----------------------------------------------------------------------
void em400_debuger_wu_sregs(unsigned int wid)
{
	em400_debuger_c_sregs(wid);
}

// -----------------------------------------------------------------------
void em400_debuger_wu_cmd(unsigned int wid)
{
}

// -----------------------------------------------------------------------
void em400_debuger_wu_status(unsigned int wid)
{
	char *kb = int2bin(R(R_KB), 16);
	awxyprint(wid, 0, 0, C_ILABEL, "  Q:");
	awprint(wid, C_IDATA, "%i", SR_Q);
	awprint(wid, C_ILABEL, "  NB:");
	awprint(wid, C_IDATA, "%i", SR_NB);
	awprint(wid, C_ILABEL, "  IC:");
	awprint(wid, C_IDATA, "0x%04x", R(R_IC));
	awprint(wid, C_ILABEL, "  P:");
	awprint(wid, C_IDATA, "%i", P);
	awprint(wid, C_ILABEL, "  MOD:");
	awprint(wid, C_IDATA, "%06i (0x%04x)", R(R_MOD), R(R_MOD));
	awprint(wid, C_ILABEL, "  MODcnt:");
	awprint(wid, C_IDATA, "%i", MODcnt);
	awprint(wid, C_ILABEL, "  KB:");
	awprint(wid, C_IDATA, "%s", kb);
	awprint(wid, C_ILABEL, "  ZC17:");
	awprint(wid, C_IDATA, "%i", ZC17);
	free(kb);
}

// vim: tabstop=4
