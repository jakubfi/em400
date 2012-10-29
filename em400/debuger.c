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

#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <signal.h>
#include <string.h>

#include "cpu.h"
#include "registers.h"
#include "memory.h"
#include "dasm.h"
#include "errors.h"
#include "utils.h"
#include "debuger.h"
#include "debuger_ui.h"
#include "debuger_parser.h"

extern int em400_quit;

char *debuger_prompt;
int nc_repaint = 0;
int debuger_fin = 0;

extern int yyparse();
typedef struct yy_buffer_state *YY_BUFFER_STATE;
YY_BUFFER_STATE yy_scan_string(char *yy_str);

// -----------------------------------------------------------------------
cmd_s em400_debuger_commands[] = {
	{ "quit",	F_QUIT,		"Quit the emulator", "  quit" },
	{ "step",	F_STEP,		"Execute instruction at IC", "  step" },
	{ "help",	F_HELP,		"Print help", "  help" },
	{ "regs",	F_REGS,		"Show user registers", "  regs" },
	{ "sregs",	F_SREGS,	"Show system registers", "  sregs" },
	{ "reset",	F_RESET,	"Reset the emulator", "  reset" },
	{ "dasm",	F_DASM,		"Disassembler", "  dasm [[start] count]" },
	{ "trans",	F_TRANS,	"Translator", "  trans [[start] count]" },
	{ "mem",	F_MEM,		"Show memory contents", "  mem [block:] start-end" },
	{ "clmem",	F_CLMEM,	"Clear memory contents", "  clmem" },
	{ "load",	F_LOAD,		"Load memory image", "  load image [mem_block]" },
	{ "memcfg",	F_MEMCFG,	"Show memory configuration", "  memcfg" },
	{ NULL,		0,			NULL }
};

// -----------------------------------------------------------------------
int debuger_is_cmd(char *cmd)
{
	cmd_s* c = em400_debuger_commands;
	while (c->cmd) {
		if (!strcmp(cmd, c->cmd)) {
			return c->tok;
		}
		c++;
	}
	return 0;
}

// -----------------------------------------------------------------------
void em400_debuger_c_load(WINDOW *win, char* image, int bank)
{
	if (em400_mem_load_image(image, bank)) {
		waprintw(win, attr[C_ERROR], "Cannot load image: \"%s\"\n", image);
	}
}

// -----------------------------------------------------------------------
void em400_debuger_c_help(WINDOW *win, char *cmd)
{
	cmd_s *c = em400_debuger_commands;
	if (cmd) {
		while (c->cmd) {
			if (!strcmp(cmd, c->cmd)) {
				waprintw(win, attr[C_LABEL], "%s : ", c->cmd);
				waprintw(win, attr[C_DATA], "%s\n", c->doc);
				waprintw(win, attr[C_LABEL], "Usage:\n%s\n", c->help);
				return;
			}
			c++;
		}
		waprintw(win, attr[C_ERROR], "Error: no such command: %s\n", cmd);
	} else {
		while (c->cmd) {
			waprintw(win, attr[C_LABEL], "%-10s : %s\n", c->cmd, c->doc);
			c++;
		}
	}
}

// -----------------------------------------------------------------------
void em400_debuger_c_quit()
{
	debuger_fin = 1;
	em400_quit = 1;
}

// -----------------------------------------------------------------------
void em400_debuger_c_step()
{
	debuger_fin = 1;
}

// -----------------------------------------------------------------------
void em400_debuger_c_reset()
{
	mjc400_reset();
}

// -----------------------------------------------------------------------
void em400_debuger_c_clmem()
{
	em400_mem_clear();
}

// -----------------------------------------------------------------------
void em400_debuger_c_dt(WINDOW *win, int dasm_mode, int start, int count)
{
	char *buf;
	int len;

	while (count > 0) {
		len = mjc400_dt(em400_mem_ptr(SR_Q * SR_NB, start), &buf, dasm_mode);

		if (start == R(R_IC)) {
			waprintw(win, attr[C_ILABEL], "0x%04x:", start);
			waprintw(win, attr[C_IDATA], " %-19s\n", buf);
		} else {
			waprintw(win, attr[C_LABEL], "0x%04x:", start);
			waprintw(win, attr[C_DATA], " %-19s\n", buf);
		}

		start += len;
		count--;
		free(buf);
	}
}

// -----------------------------------------------------------------------
void em400_debuger_c_mem(WINDOW *win, int block, int start, int end)
{
	uint16_t addr = start;
	char *text = malloc(MEMDUMP_COLS*2+1);
	char *tptr = text;
	char c1, c2;
	uint16_t *blockptr;

	blockptr = em400_mem_ptr(block, 0);
	if (!blockptr) {
		waprintw(WCMD, attr[C_ERROR], "Cannot access block %i\n", block);
	}

	// wrong range
	if ((end >= 0) && (start > end)) {
		waprintw(WCMD, attr[C_ERROR], "Wrong memory range: %i - %i\n", start, end);
	}

	// only start position given, adjust end position
	if (end < 0) {
		end = start;
	}

	// print headers, mind MEMDUMP_COLS
	waprintw(win, attr[C_LABEL], "  addr: ");
	for (int i=0 ; i<MEMDUMP_COLS ; i++) {
		waprintw(win, attr[C_LABEL], "+%03x ", i);
	}
	waprintw(win, attr[C_LABEL], "\n");
	// print separator
	waprintw(win, attr[C_LABEL], "-------");
	for (int i=0 ; i<MEMDUMP_COLS ; i++) {
		waprintw(win, attr[C_LABEL], "-----");
	}
	waprintw(win, attr[C_LABEL], "  ");
	for (int i=0 ; i<MEMDUMP_COLS ; i++) {
		waprintw(win, attr[C_LABEL], "--");
	}
	waprintw(win, attr[C_LABEL], "\n");

	// print row
	while (addr <= end) {
		// row header
		if ((addr-start)%MEMDUMP_COLS == 0) {
			waprintw(win, attr[C_LABEL], "0x%04x: ", addr); 
		}

		// hex contents
		waprintw(win, attr[C_DATA], "%4x ", *(blockptr+addr));

		// store text representation
		c1 = (char) (((*(blockptr+addr))&0b111111110000000)>>8);
		if ((c1<32)||((c1>126)&&(c1<160))) c1 = '.';
		c2 = (char) ((*(blockptr+addr))&0b0000000011111111);
		if ((c2<32)||((c2>126)&&(c2<160))) c2 = '.';
		tptr += sprintf(tptr, "%c%c", c1, c2);

		// row footer - text representation
		if ((addr-start)%MEMDUMP_COLS == (MEMDUMP_COLS-1)) {
			waprintw(win, attr[C_DATA], " %s\n", text);
			tptr = text;
		}

		addr++;
	}

	// fill and finish current line
	if ((addr-start-1)%MEMDUMP_COLS != (MEMDUMP_COLS-1)) {
		while ((addr-start)%MEMDUMP_COLS !=0) {
			waprintw(win, attr[C_DATA], "     ");
			addr++;
		}
		waprintw(win, attr[C_DATA], " %s\n", text);
	}

	free(text);
}

// -----------------------------------------------------------------------
void em400_debuger_c_sregs(WINDOW *win)
{
	char *ir = int2bin(R(R_IR)>>10, 6);
	int d = (R(R_IR)>>9) & 1;
	char *a = int2bin(R(R_IR)>>6, 3);
	char *b = int2bin(R(R_IR)>>3, 3);
	char *c = int2bin(R(R_IR), 3);

	char *rm = int2bin(R(R_SR)>>6, 10);
	int s = (R(R_SR)>>6) & 1;
	char *nb = int2bin(R(R_SR), 4);

	char *i1 = int2bin(RZ>>27, 5);
	char *i2 = int2bin(RZ>>20, 7);
	char *i3 = int2bin(RZ>>18, 2);
	char *i4 = int2bin(RZ>>16, 2);
	char *i5 = int2bin(RZ>>10, 6);
	char *i6 = int2bin(RZ>>4, 6);
	char *i7 = int2bin(RZ, 4);

	char *sf = int2bin(R(0)>>8, 8);
	char *uf = int2bin(R(0), 8);

	waprintw(win, attr[C_LABEL], "            OPCODE D A   B   C\n");
	waprintw(win, attr[C_LABEL], "IR: ");
	waprintw(win, attr[C_DATA], "0x%04x  %s %i %s %s %s\n", R(R_IR), ir, d, a, b, c);
	waprintw(win, attr[C_LABEL], "            RM         Q s NB\n");
	waprintw(win, attr[C_LABEL], "SR: ");
	waprintw(win, attr[C_DATA], "0x%04x  %s %i %i %s\n", R(R_SR), rm, SR_Q, s, nb);
	waprintw(win, attr[C_LABEL], "                ZPMCZ TIFFFFx 01 23 456789 abcdef OCSS\n");
	waprintw(win, attr[C_LABEL], "RZ: ");
	waprintw(win, attr[C_DATA], "0x%08x  %s %s %s %s %s %s %s\n", RZ, i1, i2, i3, i4, i5, i6, i7);
	waprintw(win, attr[C_LABEL], "            ZMVCLEGY Xuser\n");
	waprintw(win, attr[C_LABEL], "R0: ");
	waprintw(win, attr[C_DATA], "0x%04x  %s %s\n", R(0), sf, uf);

	free(uf);
	free(sf);

	free(i1);
	free(i2);
	free(i3);
	free(i4);
	free(i5);
	free(i6);
	free(i7);

	free(rm);
	free(nb);

	free(ir);
	free(a);
	free(b);
	free(c);
}

// -----------------------------------------------------------------------
void em400_debuger_c_regs(WINDOW *win)
{
	waprintw(win, attr[C_LABEL], "    hex    oct    dec    bin              ch R40\n");
	for (int i=1 ; i<=7 ; i++) {
		char *b = int2bin(R(i), 16);
		char *r = int2r40(R(i));
		char *c = int2chars(R(i));

		waprintw(win, attr[C_LABEL], "R%i: ", i);
		waprintw(win, attr[C_DATA], "0x%04x %6o %6i %s %s %s\n", R(i), R(i), (int16_t)R(i), b, c, r);
		free(c);
		free(r);
		free(b);
	}
}

// -----------------------------------------------------------------------
void em400_debuger_c_memcfg(WINDOW *win)
{
	int i, j, cnt;
	waprintw(win, attr[C_LABEL], "Number of 4kword segments in each segment/block\n");
	waprintw(win, attr[C_LABEL], "seg/blk:  0 1 2 3 4 5 6 7 8 9 a b c d e f\n");
	waprintw(win, attr[C_LABEL], "     hw:  ");
	for (i=0 ; i<MEM_MAX_MODULES ; i++) {
		cnt = 0;
		for (j=0 ; j<MEM_MAX_SEGMENTS ; j++) {
			if (em400_mem_segment[i][j]) cnt++;
		}
		waprintw(win, attr[C_DATA], "%i ", cnt);
	}
	waprintw(win, attr[C_DATA], "\n");
	waprintw(win, attr[C_LABEL], "     sw:  ");
	for (i=0 ; i<MEM_MAX_NB ; i++) {
		cnt = 0;
		for (j=0 ; j<MEM_MAX_SEGMENTS ; j++) {
			if (em400_mem_map[i][j]) cnt++;
		}
		waprintw(win, attr[C_DATA], "%i ", cnt);
	}
	waprintw(win, attr[C_DATA], "\n");
}

// -----------------------------------------------------------------------
int em400_debuger_init()
{
	nc_w_changed = 1;
	return em400_debuger_ui_init();
}

// -----------------------------------------------------------------------
void em400_debuger_shutdown()
{
	em400_debuger_ui_shutdown();
}

// -----------------------------------------------------------------------
void em400_debuger_loop()
{
	int nbufsize = 70;
	char buf[nbufsize];
	int res;

	debuger_fin = 0;

	while (!debuger_fin) {
		if (nc_w_changed) {
			e400_debuger_w_reinit_all();
			nc_w_changed = 0;
		} else {
			e400_debuger_w_redraw_all();
		}

		res = nc_readline(WCMD, "em400> ", buf, nbufsize);
		waprintw(WCMD, 0, "\n");

		if ((res == KEY_ENTER) && (*buf)) {
			yy_scan_string(buf);
			yyparse();
		}
	}
}

// vim: tabstop=4
