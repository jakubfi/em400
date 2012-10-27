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
	{ "regs",	F_REGS,		"Show registers", "  regs" },
	{ "reset",	F_RESET,	"Reset the emulator", "  reset" },
	{ "dasm",	F_DASM,		"Disassembler", "  dasm\n  dasm count\n  dasm start count" },
	{ "trans",	F_TRANS,	"Translator", "  trans\n  trans count\n  trans start count" },
	{ "mem",	F_MEM,		"Show memory contents (any block)", "  mem block word_addr\n  mem block start_addr end_addr",  },
	{ "memq",	F_MEM,		"Show memory contents (block by Q,NB)", "  memq word_addr\n  memq start_addr end_addr" },
	{ "memnb",	F_MEM,		"Show memory contents (block by NB)", "  memnb word_addr\n  memnb start_addr end_addr" },
	{ "clmem",	F_CLMEM,	"Clear memory contents", "  clmem" },
	{ "load",	F_LOAD,		"Load memory image", "  load image mem_block" },
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
	if (bank < 0) {
		bank = SR_NB;
	}

	if (em400_mem_load_image(image, bank)) {
		wprintw(win, "Cannot load image: \"%s\"\n", image);
	}
}

// -----------------------------------------------------------------------
void em400_debuger_c_help(WINDOW *win, int cmd_tok)
{
	cmd_s *c = em400_debuger_commands;
	if (cmd_tok) {
		while (c->cmd) {
			if (cmd_tok == c->tok) {
				wprintw(win, "%s : %s\nUsage:\n%s\n", c->cmd, c->doc, c->help);
				return;
			}
			c++;
		}
	} else {
		while (c->cmd) {
			wprintw(win, "%-10s : %s\n", c->cmd, c->doc);
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

	if (start < 0) {
		start = R(R_IC);
	}

	while (count > 0) {
		len = mjc400_dt(em400_mem_ptr(SR_Q*SR_NB, start), &buf, dasm_mode);
		if (start == R(R_IC)) {
			wattrset(win, attr[C_ILABEL]);
		} else {
			wattrset(win, attr[C_LABEL]);
		}
		wprintw(win, "0x%04x:", start);
		if (start == R(R_IC)) {
			wattrset(win, attr[C_IDATA]);
		} else {
			wattrset(win, attr[C_DATA]);
		}
		wprintw(win, " %-19s\n", buf);
		start += len;
		count--;
		free(buf);
	}
	wattroff(win, attr);
}

// -----------------------------------------------------------------------
void em400_debuger_c_mem(WINDOW *win, int block, int start, int end)
{
	uint16_t addr = start;
	char *text = malloc(MEMDUMP_COLS*2+1);
	char *tptr = text;
	char c1, c2;
	uint16_t *blockptr;

	if (block < 0) {
		if (SR_Q) block = SR_NB;
		else block = 0;
	}

	blockptr = em400_mem_ptr(block, 0);
	if (!blockptr) {
		wprintw(WCMD, "Cannot access block %i\n", block);
	}

	// wrong range
	if ((end >= 0) && (start > end)) {
		wprintw(WCMD, "Wrong memory range: %i - %i\n", start, end);
	}

	// only start position given, adjust end position
	if (end < 0) {
		end = start;
	}

	// print headers, mind MEMDUMP_COLS
	wprintw(win, "  addr: ");
	for (int i=0 ; i<MEMDUMP_COLS ; i++) {
		wprintw(win, "+%03x ", i);
	}
	wprintw(win, "\n");
	// print separator
	wprintw(win, "-------");
	for (int i=0 ; i<MEMDUMP_COLS ; i++) {
		wprintw(win, "-----");
	}
	wprintw(win, "  ");
	for (int i=0 ; i<MEMDUMP_COLS ; i++) {
		wprintw(win, "--");
	}
	wprintw(win, "\n");

	// print row
	while (addr <= end) {
		// row header
		if ((addr-start)%MEMDUMP_COLS == 0) {
			wprintw(win, "0x%04x: ", addr); 
		}

		// hex contents
		wprintw(win, "%4x ", *(blockptr+addr));

		// store text representation
		c1 = (char) (((*(blockptr+addr))&0b111111110000000)>>8);
		if ((c1<32)||((c1>126)&&(c1<160))) c1 = '.';
		c2 = (char) ((*(blockptr+addr))&0b0000000011111111);
		if ((c2<32)||((c2>126)&&(c2<160))) c2 = '.';
		tptr += sprintf(tptr, "%c%c", c1, c2);

		// row footer - text representation
		if ((addr-start)%MEMDUMP_COLS == (MEMDUMP_COLS-1)) {
			wprintw(win, " %s\n", text);
			tptr = text;
		}

		addr++;
	}

	// fill and finish current line
	if ((addr-start-1)%MEMDUMP_COLS != (MEMDUMP_COLS-1)) {
		while ((addr-start)%MEMDUMP_COLS !=0) {
			wprintw(win, "     ");
			addr++;
		}
		wprintw(win, " %s\n", text);
	}

	free(text);
}

// -----------------------------------------------------------------------
void em400_debuger_c_regs(WINDOW *win)
{
	char *ir = int2bin(R(R_IR), 16);
	char *sr = int2bin(R(R_SR), 16);
	char *rz = int2bin(RZ, 32);
	char *r0 = int2bin(R(0), 16);

	wprintw(win, "           iiiiiiDAAABBBCCC             RM________QBNB__\n");
	wprintw(win, "IR: 0x%04x %s  SR: 0x%04x %s\n", R(R_IR), ir, R(R_SR), sr);
	wprintw(win, "IC: 0x%04x P: %i\n", R(R_IC), P);
	wprintw(win, "\n");
	wprintw(win, "    01234567012345670123456701234567      ZMVCLEGYX.......\n");
	wprintw(win, "RZ: %s  R0: %s\n", rz, r0);
	wprintw(win, "\n");
	free(ir);
	free(sr);
	free(rz);
	free(r0);
	wprintw(win, "     hex... dec...  bin.....|.......       hex... dec...  bin.....|.......\n");
	for (int i=0 ; i<4 ; i++) {
		char *r1 = int2bin(R(2*i), 16);
		char *r2 = int2bin(R(2*i+1), 16);
		wprintw(win, "R%02i: 0x%04x  %5i  %s", 2*i, R(2*i), R(2*i), r1);
		wprintw(win, "  R%02i: 0x%04x  %5i  %s\n", 2*i+1, R(2*i+1), R(2*i+1), r2);
		free(r1);
		free(r2);
	}
	wprintw(win, "\n");
	wprintw(win, "MOD: 0x%04x %i  MODcnt: %i  P: %i  ZC17: %i\n", (uint16_t) R(R_MOD), R(R_MOD), MODcnt, P, ZC17);
}

// -----------------------------------------------------------------------
int em400_debuger_init()
{
	initscr();
	cbreak();
	noecho();
	start_color();
	em400_debuger_ui_init();

	struct sigaction sa;
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = _em400_debuger_w_resize_sig;

	if (sigemptyset(&sa.sa_mask) != 0) {
		return E_DEBUGER_SIG_RESIZE;
	}

	if (sigaction(SIGWINCH, &sa, NULL) != 0) {
		return E_DEBUGER_SIG_RESIZE;
	}

	e400_debuger_w_reinit_all();

	return E_OK;
}

// -----------------------------------------------------------------------
void em400_debuger_shutdown()
{
	endwin();
}

// -----------------------------------------------------------------------
void em400_debuger_loop()
{
	int nbufsize = 1024;
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
		wprintw(WCMD, "\n");

		if ((res == KEY_ENTER) && (*buf)) {
			yy_scan_string(buf);
			yyparse();
		}
	}
}

// vim: tabstop=4
