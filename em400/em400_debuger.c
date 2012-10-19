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

#include "em400_errors.h"
#include "em400_utils.h"
#include "em400_debuger.h"
#include "mjc400_regs.h"
#include "mjc400.h"
#include "mjc400_dasm.h"
#include "em400_mem.h"
#include "em400_debuger_nc.h"

char *debuger_prompt;

// -----------------------------------------------------------------------
cmd_s em400_debuger_commands[] = {
	{ "exit",	em400_debuger_c_quit,	"Quit the emulator", "  exit" },
	{ "quit",	em400_debuger_c_quit,	"Quit the emulator", "  quit" },
	{ "step",	em400_debuger_c_step,	"Execute instruction at IC", "  step" },
	{ "help",	em400_debuger_c_help,	"Print help", "  help" },
	{ "?",		em400_debuger_c_help,	"Print help", "  ?" },
	{ "regs",	em400_debuger_c_regs,	"Show registers", "  regs" },
	{ "reset",	em400_debuger_c_reset,	"Reset the emulator", "  reset" },
	{ "dasm",	em400_debuger_c_dasm,	"Disassembler", "  dasm\n  dasm count\n  dasm start count" },
	{ "trans",	em400_debuger_c_trans,	"Translator", "  trans\n  trans count\n  trans start count" },
	{ "mem",	em400_debuger_c_mem,	"Show memory contents (any block)", "  mem block word_addr\n  mem block start_addr end_addr",  },
	{ "memq",	em400_debuger_c_memq,	"Show memory contents (block by Q,NB)", "  memq word_addr\n  memq start_addr end_addr" },
	{ "memnb",	em400_debuger_c_memnb,	"Show memory contents (block by NB)", "  memnb word_addr\n  memnb start_addr end_addr" },
	{ "clmem",	em400_debuger_c_clmem,	"Clear memory contents", "  clmem" },
	{ "load",	em400_debuger_c_load,	"Load memory image", "  load image mem_block" },
	{ "save",	em400_debuger_c_save,	"Load save image", "  save image mem_block" },
	{ NULL,		NULL,			NULL, NULL }
};

// -----------------------------------------------------------------------
int em400_debuger_c_quit(char* args)
{
	return DEBUGER_EM400_QUIT;
}

// -----------------------------------------------------------------------
int em400_debuger_c_step(char* args)
{
	return DEBUGER_EM400_STEP;
}

// -----------------------------------------------------------------------
int em400_debuger_c_load(char* args)
{
	char *image = malloc(256+1);
	int bank = -1;

	int n = sscanf(args, "%256s %i", image, &bank);

	// parse error
	if ((n<2) || (n>2)) {
		return DEBUGER_LOOP_ERR;
	}

	if (em400_mem_load_image(image, bank)) {
		printw("Cannot load image: \"%s\"\n", image);
		return DEBUGER_LOOP_ERR;
	}

	free(image);

	return DEBUGER_EM400_SKIP;
}

// -----------------------------------------------------------------------
int em400_debuger_c_save(char* args)
{
	return DEBUGER_EM400_SKIP;
}

// -----------------------------------------------------------------------
int em400_debuger_c_help(char* args)
{
	cmd_s *c = em400_debuger_commands;
	if (args && *args) {
		while (c->cmd) {
			if (!strcmp(args, c->cmd)) {
				printw("%s : %s\nUsage:\n%s\n", c->cmd, c->doc, c->help);
				return DEBUGER_EM400_SKIP;
			}
			c++;
		}
		return DEBUGER_LOOP_ERR;
	} else {
		while (c->cmd) {
			printw("%-10s : %s\n", c->cmd, c->doc);
			c++;
		}
	}
	return DEBUGER_EM400_SKIP;
}

// -----------------------------------------------------------------------
int em400_debuger_c_reset(char* args)
{
	mjc400_reset();
	return DEBUGER_EM400_SKIP;
}

// -----------------------------------------------------------------------
int em400_debuger_c_clmem(char* args)
{
	em400_mem_clear();
	return DEBUGER_EM400_SKIP;
}

// -----------------------------------------------------------------------
int __em400_debuger_c_dt(char* args, int dasm_mode)
{
	int d_start;
	int d_count;

	int n = sscanf(args, "%i %i", &d_start, &d_count);

	if (n <= 0) {
		d_count = 1;
		d_start = IC;
	} else if (n == 1) {
		d_count = d_start;
		d_start = IC;
	} else if (n == 2) {
	} else {
		return DEBUGER_LOOP_ERR;
	}

	char *buf;
	int len;

	while (d_count > 0) {
		len = mjc400_dt(em400_mem_ptr(SR_Q*SR_NB, d_start), &buf, dasm_mode);
		printw("0x%04x:\t%s\n", d_start, buf);
		d_start += len;
		d_count--;
		free(buf);
	}
	return DEBUGER_EM400_SKIP;
}

// -----------------------------------------------------------------------
int em400_debuger_c_dasm(char* args)
{
	return __em400_debuger_c_dt(args, DMODE_DASM);
}

// -----------------------------------------------------------------------
int em400_debuger_c_trans(char* args)
{
	return __em400_debuger_c_dt(args, DMODE_TRANS);
}

// -----------------------------------------------------------------------
int __em400_debuger_dump_mem(int block, int start, int end)
{
	uint16_t addr = start;
	char *text = malloc(MEMDUMP_COLS*2+1);
	char *tptr = text;
	char c1, c2;
	uint16_t *blockptr;

	blockptr = em400_mem_ptr(block, 0);
	if (!blockptr) {
		return DEBUGER_LOOP_ERR;
	}

	// wrong range
	if ((end >= 0) && (start > end)) {
		return DEBUGER_LOOP_ERR;
	}

	// only start position given, adjust end position
	if (end < 0) {
		end = start;
	}

	// print headers, mind MEMDUMP_COLS
	printw("  addr: ");
	for (int i=0 ; i<MEMDUMP_COLS ; i++) {
		printw("+%03x ", i);
	}
	printw("\n");
	// print separator
	printw("-------");
	for (int i=0 ; i<MEMDUMP_COLS ; i++) {
		printw("-----");
	}
	printw("  ");
	for (int i=0 ; i<MEMDUMP_COLS ; i++) {
		printw("--");
	}
	printw("\n");

	// print row
	while (addr <= end) {
		// row header
		if ((addr-start)%MEMDUMP_COLS == 0) {
			printw("0x%04x: ", addr); 
		}

		// hex contents
		printw("%4x ", *(blockptr+addr));

		// store text representation
		c1 = (char) (((*(blockptr+addr))&0b111111110000000)>>8);
		if ((c1<32)||((c1>126)&&(c1<160))) c1 = '.';
		c2 = (char) ((*(blockptr+addr))&0b0000000011111111);
		if ((c2<32)||((c2>126)&&(c2<160))) c2 = '.';
		tptr += sprintf(tptr, "%c%c", c1, c2);

		// row footer - text representation
		if ((addr-start)%MEMDUMP_COLS == (MEMDUMP_COLS-1)) {
			printw(" %s\n", text);
			tptr = text;
		}

		addr++;
	}

	// fill and finish current line
	if ((addr-start-1)%MEMDUMP_COLS != (MEMDUMP_COLS-1)) {
		while ((addr-start)%MEMDUMP_COLS !=0) {
			printw("     ");
			addr++;
		}
		printw(" %s\n", text);
	}

	free(text);

	return DEBUGER_EM400_SKIP;
}

// -----------------------------------------------------------------------
int em400_debuger_c_memq(char* args)
{
	int m_start = -1;
	int m_end = -1;
	int n = sscanf(args, "%i %i", &m_start, &m_end);

	// parse error
	if ((n<1) || (n>2)) {
		return DEBUGER_LOOP_ERR;
	}

	return __em400_debuger_dump_mem(SR_Q*SR_NB, m_start, m_end);
}

// -----------------------------------------------------------------------
int em400_debuger_c_memnb(char* args)
{
	int m_start = -1;
	int m_end = -1;
	int n = sscanf(args, "%i %i", &m_start, &m_end);

	// parse error
	if ((n<1) || (n>2)) {
		return DEBUGER_LOOP_ERR;
	}

	return __em400_debuger_dump_mem(SR_NB, m_start, m_end);
}

// -----------------------------------------------------------------------
int em400_debuger_c_mem(char* args)
{
	int m_block = -1;
	int m_start = -1;
	int m_end = -1;
	int n = sscanf(args, "%i %i %i", &m_block, &m_start, &m_end);

	// parse error
	if ((n<=1) || (n>3)) {
		return DEBUGER_LOOP_ERR;
	}

	return __em400_debuger_dump_mem(m_block, m_start, m_end);
}

// -----------------------------------------------------------------------
int em400_debuger_c_regs(char* args)
{
	char *ir = int2bin(IR, 16);
	char *sr = int2bin(SR, 16);
	char *rz = int2bin(RZ, 32);
	char *r0 = int2bin(R[0], 16);

	printw("           iiiiiiDAAABBBCCC             RM________QBNB__\n");
	printw("IR: 0x%04x %s  SR: 0x%04x %s\n", IR, ir, SR, sr);
	printw("IC: 0x%04x P: %i\n", IC, P);
	printw("\n");
	printw("    01234567012345670123456701234567      ZMVCLEGYX.......\n");
	printw("RZ: %s  R0: %s\n", rz, r0);
	printw("\n");
	free(ir);
	free(sr);
	free(rz);
	free(r0);
	printw("     hex... dec...  bin.....|.......       hex... dec...  bin.....|.......\n");
	for (int i=0 ; i<4 ; i++) {
		char *r1 = int2bin(R[2*i], 16);
		char *r2 = int2bin(R[2*i+1], 16);
		printw("R%02i: 0x%04x  %5i  %s", 2*i, R[2*i], R[2*i], r1);
		printw("  R%02i: 0x%04x  %5i  %s\n", 2*i+1, R[2*i+1], R[2*i+1], r2);
		free(r1);
		free(r2);
	}
	printw("\n");
	printw("MOD: 0x%04x %i  MODcnt: %i  P: %i  ZC17: %i\n", (uint16_t) MOD, MOD, MODcnt, P, ZC17);
	return DEBUGER_EM400_SKIP;
}

// -----------------------------------------------------------------------
int em400_debuger_execute(char* line)
{
	char cmd[10+1] = {0};
	char args[100+1] = {0};
	cmd_s* c = em400_debuger_commands;

	int n;
	n = sscanf(line, "%s %100c", cmd, args);
	if (n <= 0) {
		return DEBUGER_EM400_SKIP;
	}

	while (c->cmd) {
		if (!strcmp(cmd, c->cmd)) {
			int ret;
			ret = c->fun(args);
			if (ret == DEBUGER_LOOP_ERR) {
				printw("Error while processing command. Usage:\n%s\n", c->help);
			}
			return ret;
		}
		c++;
	}

	printw("Unknown command: '%s'. Try 'help'.\n", cmd);
	return DEBUGER_EM400_SKIP;
}

// -----------------------------------------------------------------------
void _em400_debuger_resize_sig(int signum, siginfo_t *si, void *ctx)
{

}

// -----------------------------------------------------------------------
int em400_debuger_init()
{
	WINDOW * w_main = initscr();
	scrollok(w_main, TRUE);
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	start_color();

	init_pair(1, COLOR_YELLOW, COLOR_BLACK);

	struct sigaction sa;
	sa.sa_flags = SA_SIGINFO | SA_RESTART;
	sa.sa_sigaction = _em400_debuger_resize_sig;

	if (sigemptyset(&sa.sa_mask) != 0) {
		return E_DEBUGER_INIT;
	}

	if (sigaction(SIGWINCH, &sa, NULL) != 0) {
		return E_DEBUGER_INIT;
	}

	return E_OK;
}

// -----------------------------------------------------------------------
void em400_debuger_shutdown()
{
	endwin();
}

// -----------------------------------------------------------------------
int em400_debuger_step()
{
	int ret = DEBUGER_EM400_STEP;
	int nbufsize = 1024;
	char buf[nbufsize];

	nc_readline("em400> ", buf, nbufsize);

	if (*buf) {
		ret = em400_debuger_execute(buf);
	} else {
		ret = DEBUGER_EM400_SKIP;
	}

	return ret;
}

// vim: tabstop=4
