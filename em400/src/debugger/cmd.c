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

#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "registers.h"
#include "interrupts.h"
#include "memory.h"
#include "errors.h"
#include "utils.h"

#include "debugger/awin.h"
#include "debugger/dasm.h"
#include "debugger/debugger.h"
#include "debugger/cmd.h"
#include "debugger/ui.h"
#include "parser.h"
#include "debugger/eval.h"

extern int em400_quit;

// -----------------------------------------------------------------------
struct cmd_t dbg_commands[] = {
	{ "quit",	F_QUIT,		"Quit the emulator", "  quit" },
	{ "step",	F_STEP,		"Execute instruction at IC", "  step" },
	{ "help",	F_HELP,		"Print help", "  help" },
	{ "regs",	F_REGS,		"Show user registers", "  regs" },
	{ "sregs",	F_SREGS,	"Show system registers", "  sregs" },
	{ "reset",	F_RESET,	"Reset the emulator", "  reset" },
	{ "dasm",	F_DASM,		"Disassembler", "  dasm [[start] count]" },
	{ "trans",	F_TRANS,	"Translator", "  trans [[start] count]" },
	{ "mem",	F_MEM,		"Show memory contents", "  mem [block:] <start>-<end>" },
	{ "clmem",	F_CLMEM,	"Clear memory contents", "  clmem" },
	{ "load",	F_LOAD,		"Load memory image from file", "  load <file> [mem_block]" },
	{ "memcfg",	F_MEMCFG,	"Show memory configuration", "  memcfg" },
	{ "brk",	F_BRK,		"Manipulate breakpoints", "  brk add <expression>\n  brk list\n  brk del <brk_number>" },
	{ "run",	F_RUN,		"Run emulation", "  run" },
	{ "stack",	F_STACK,	"Show stack", "  stack" },
	{ NULL,		0,			NULL }
};

// -----------------------------------------------------------------------
int dbg_is_cmd(char *cmd)
{
	struct cmd_t* c = dbg_commands;
	while (c->cmd) {
		if (!strcmp(cmd, c->cmd)) {
			return c->tok;
		}
		c++;
	}
	return 0;
}

// -----------------------------------------------------------------------
void dbg_c_load(int wid, char* image, int bank)
{
	if (mem_load_image(image, bank)) {
		awprint(wid, C_ERROR, "Cannot load image: \"%s\"\n", image);
	}
}

// -----------------------------------------------------------------------
void dbg_c_help(int wid, char *cmd)
{
	struct cmd_t *c = dbg_commands;
	if (cmd) {
		while (c->cmd) {
			if (!strcmp(cmd, c->cmd)) {
				awprint(wid, C_DATA, "%s ", c->cmd);
				awprint(wid, C_LABEL, ": %s\n", c->doc);
				awprint(wid, C_LABEL, "Usage:\n%s\n", c->help);
				return;
			}
			c++;
		}
		awprint(wid, C_ERROR, "Error: no such command: %s\n", cmd);
	} else {
		while (c->cmd) {
			awprint(wid, C_LABEL, "%-10s : %s\n", c->cmd, c->doc);
			c++;
		}
	}
}

// -----------------------------------------------------------------------
void dbg_c_quit()
{
	em400_quit = 1;
	dbg_loop_fin = 1;
}

// -----------------------------------------------------------------------
void dbg_c_step()
{
	dbg_enter = 1;
	dbg_loop_fin = 1;
}

// -----------------------------------------------------------------------
void dbg_c_run()
{
	dbg_enter = 0;
	dbg_loop_fin = 1;
}

// -----------------------------------------------------------------------
void dbg_c_reset()
{
	cpu_reset();
	mem_actr_max = -1;
	mem_actw_max = -1;
	for (int i=0 ; i<R_MAX ; i++) {
		reg_act[i] = C_DATA;
	}
}

// -----------------------------------------------------------------------
void dbg_c_clmem()
{
	mem_clear();
}

// -----------------------------------------------------------------------
void dbg_c_dt(int wid, int dasm_mode, int start, int count)
{
	char *buf;
	int len;

	while (count > 0) {
		uint16_t * addr = mem_ptr(SR_Q * SR_NB, start);
		if (addr) {
			len = dt_trans(addr, &buf, dasm_mode);

			if (start == regs[R_IC]) {
				awprint(wid, C_ILABEL, "0x%04x:", start);
				awprint(wid, C_IDATA, " %-19s\n", buf);
			} else {
				awprint(wid, C_LABEL, "0x%04x:", start);
				awprint(wid, C_DATA, " %-19s\n", buf);
			}
			start += len;
			free(buf);
		} else {
			awprint(wid, C_DATA, "\n");
		}
		count--;
	}
}

// -----------------------------------------------------------------------
void dbg_c_mem(int wid, int block, int start, int end, int maxcols, int maxlines)
{
	uint16_t *mptr = mem_ptr(block, 0);

	if (!mptr) {
		awprint(wid, C_ERROR, "Cannot access block %i\n", block);
	}

	// wrong range
	if (end - start <= 0) {
		awprint(wid, C_ERROR, "Wrong memory range: %i - %i\n", start, end);
	}

	int words = (maxcols - 10) / 7;
	if (words>16) words = 16;
	maxlines -=2; // two used for header

	// headers
	awprint(wid, C_LABEL, "  addr: ");
	for (int i=0 ; i<words ; i++) awprint(wid, C_LABEL, "+%03x ", i);
	awprint(wid, C_LABEL, "\n");
	awprint(wid, C_LABEL, "-------");
	for (int i=0 ; i<words ; i++) awprint(wid, C_LABEL, "-----");
	awprint(wid, C_LABEL, "  ");
	for (int i=0 ; i<words ; i++) awprint(wid, C_LABEL, "--");
	awprint(wid, C_LABEL, "\n");

	uint16_t addr = start;
	int attr = C_DATA;

	while ((maxlines > 0) && (addr <= end)) {
		// row header
		awprint(wid, C_LABEL, "0x%04x: ", addr);
		char *chars = malloc(words*2+1);
		for (int w=0 ; w<words ; w++) {
			mptr = mem_ptr(block, addr);
			if (!mptr) {
				return;
			}

			// data (hex)
			if ((addr >= mem_actw_min) && (addr <= mem_actw_max)) {
				if ((addr >= mem_actr_min) && (addr <= mem_actr_max)) {
					attr = C_RW;
				} else {
					attr = C_WRITE;
				}
			} else if ((addr >= mem_actr_min) && (addr <= mem_actr_max)) {
				attr = C_READ;
			} else if (addr == regs[R_IC]) {
				attr = C_DATAU;
			} else {
				attr = C_DATA;
			}
			awprint(wid, attr, "%4x", *mptr);
			awprint(wid, C_DATA, " ");

			// store data (chars)
			int2chars(*mptr, chars+w*2);
			addr++;
		}
		// data (chars)
		awprint(wid, C_DATA, " %s\n", chars);
		free(chars);
		maxlines--;
	}
}

// -----------------------------------------------------------------------
void dbg_c_sregs(int wid)
{
	char *ir = int2bin(regs[R_IR]>>10, 6);
	int d = (regs[R_IR]>>9) & 1;
	char *a = int2bin(regs[R_IR]>>6, 3);
	char *b = int2bin(regs[R_IR]>>3, 3);
	char *c = int2bin(regs[R_IR], 3);

	char *rm = int2bin(regs[R_SR]>>6, 10);
	int s = (regs[R_SR]>>4) & 1;
	char *nb = int2bin(regs[R_SR], 4);

	char *rz1 = int2bin(RZ>>27, 5);
	char *rz2 = int2bin(RZ>>20, 7);
	char *rz3 = int2bin(RZ>>18, 2);
	char *rz4 = int2bin(RZ>>16, 2);
	char *rz5 = int2bin(RZ>>10, 6);
	char *rz6 = int2bin(RZ>>4, 6);
	char *rz7 = int2bin(RZ, 4);

	char *rp1 = int2bin(RP>>27, 5);
	char *rp2 = int2bin(RP>>20, 7);
	char *rp3 = int2bin(RP>>18, 2);
	char *rp4 = int2bin(RP>>16, 2);
	char *rp5 = int2bin(RP>>10, 6);
	char *rp6 = int2bin(RP>>4, 6);
	char *rp7 = int2bin(RP, 4);

	char *xm1 = int2bin(xmask>>27, 5);
	char *xm2 = int2bin(xmask>>20, 7);
	char *xm3 = int2bin(xmask>>18, 2);
	char *xm4 = int2bin(xmask>>16, 2);
	char *xm5 = int2bin(xmask>>10, 6);
	char *xm6 = int2bin(xmask>>4, 6);
	char *xm7 = int2bin(xmask, 4);

	char *sf = int2bin(regs[0]>>8, 8);
	char *uf = int2bin(regs[0], 8);

	awprint(wid, C_LABEL, "            OPCODE D A   B   C");
	awprint(wid, C_LABEL, "               P: ");
	awprint(wid, C_DATA, "0x%x  ", regs[R_P]);
	awprint(wid, C_DATA, "\n");
	awprint(wid, C_LABEL, "IR: ");
	awprint(wid, C_DATA, "0x%04x  %s %i %s %s %s ", regs[R_IR], ir, d, a, b, c);
	awprint(wid, C_LABEL, "           IC: ");
	awprint(wid, C_DATA, "0x%04x ", regs[R_IC]);
	awprint(wid, C_DATA, "\n");
	awprint(wid, C_LABEL, "            PMCZs139fS Q s NB");
	awprint(wid, C_LABEL, "              MOD: ");
	awprint(wid, C_DATA, "0x%04x ", regs[R_MOD]);
	awprint(wid, C_DATA, "\n");
	awprint(wid, C_LABEL, "SR: ");
	awprint(wid, C_DATA, "0x%04x  %s %i %i %s", regs[R_SR], rm, SR_Q, s, nb);
	awprint(wid, C_DATA, "\n\n");

	awprint(wid, C_LABEL, "                ZPMCZ TIFFFFx 01 23 456789 abcdef OCSS");
	awprint(wid, C_DATA, "\n");
	awprint(wid, C_LABEL, "RZ: ");
	awprint(wid, C_DATA, "0x%08x  %s %s %s %s %s %s %s", RZ, rz1, rz2, rz3, rz4, rz5, rz6, rz7);
	awprint(wid, C_DATA, "\n");

	awprint(wid, C_LABEL, "Unmasked in RM: ");
	awprint(wid, C_DATA, "%s %s %s %s %s %s %s", xm1, xm2, xm3, xm4, xm5, xm6, xm7);
	awprint(wid, C_DATA, "\n");

	awprint(wid, C_LABEL, "RP: ");
	awprint(wid, C_DATA, "0x%08x  %s %s %s %s %s %s %s", RP, rp1, rp2, rp3, rp4, rp5, rp6, rp7);
	awprint(wid, C_DATA, "\n");

	free(uf);
	free(sf);

	free(rz1);
	free(rz2);
	free(rz3);
	free(rz4);
	free(rz5);
	free(rz6);
	free(rz7);

	free(xm1);
	free(xm2);
	free(xm3);
	free(xm4);
	free(xm5);
	free(xm6);
	free(xm7);

	free(rp1);
	free(rp2);
	free(rp3);
	free(rp4);
	free(rp5);
	free(rp6);
	free(rp7);

	free(rm);
	free(nb);

	free(ir);
	free(a);
	free(b);
	free(c);
}

// -----------------------------------------------------------------------
void dbg_c_regs(int wid)
{
	awprint(wid, C_LABEL, "    hex    oct    dec    ZMVCLEGY Xuser    ch R40\n");
	for (int i=0 ; i<=7 ; i++) {
		char *b1 = int2bin(regs[i]>>8, 8);
		char *b2 = int2bin(regs[i], 8);
		char *r = int2r40(regs[i]);
		char c[3];
		int2chars(regs[i], c);

		awprint(wid, C_LABEL, "R%i: ", i);
		awprint(wid, reg_act[i], "0x%04x %6o %6i %s %s %s %s\n", regs[i], regs[i], (int16_t)regs[i], b1, b2, c, r);
		free(r);
		free(b1);
		free(b2);
	}
}

// -----------------------------------------------------------------------
void dbg_c_stack(int wid, int size)
{
	static int sb;
	static int osp;

	int sp = *mem_ptr(0, 97);

	if ((sb <= 0) || (sp-osp > 4) || (sp-osp < -4)) {
		sb = sp;
	}

	osp = sp;

	while (sp >= sb) {
		if (sp == osp) {
			awprint(wid, C_ILABEL, " 0x%04x: ", sp);
			awprint(wid, C_IDATA, "%04x \n", *mem_ptr(0, sp));
		} else {
			awprint(wid, C_LABEL, " 0x%04x: ", sp);
			awprint(wid, C_DATA, "%04x \n", *mem_ptr(0, sp));
		}
		sp--;
	}
	awprint(wid, C_LABEL, "--------------\n");
}

// -----------------------------------------------------------------------
void dbg_c_memcfg(int wid)
{
	int i, j, cnt;
	awprint(wid, C_LABEL, "Number of 4kword segments in each segment/block\n");
	awprint(wid, C_LABEL, "seg/blk:  0 1 2 3 4 5 6 7 8 9 a b c d e f\n");
	awprint(wid, C_LABEL, "     hw:  ");
	for (i=0 ; i<MEM_MAX_MODULES ; i++) {
		cnt = 0;
		for (j=0 ; j<MEM_MAX_SEGMENTS ; j++) {
			if (mem_segment[i][j]) cnt++;
		}
		awprint(wid, C_DATA, "%i ", cnt);
	}
	awprint(wid, C_DATA, "\n");
	awprint(wid, C_LABEL, "     sw:  ");
	for (i=0 ; i<MEM_MAX_NB ; i++) {
		cnt = 0;
		for (j=0 ; j<MEM_MAX_SEGMENTS ; j++) {
			if (mem_map[i][j]) cnt++;
		}
		awprint(wid, C_DATA, "%i ", cnt);
	}
	awprint(wid, C_DATA, "\n");
}

// -----------------------------------------------------------------------
void dbg_c_brk_add(int wid, char *label, struct node_t *n)
{
	static int brkcnt;

	struct break_t *b = malloc(sizeof(struct break_t));
	b->nr = brkcnt++;
	b->counter = 0;
	b->disabled = 0;
	b->label = strdup(label);
	b->n = n;
	b->next = NULL;

	if (brk_last) {
		brk_last->next = b;
		brk_last = b;
	} else {
		brk_last = brk_stack = b;
	}

	awprint(wid, C_LABEL, "Breakpoint ");
	awprint(wid, C_DATA, "%i", b->nr);
	awprint(wid, C_LABEL, " added: \"");
	awprint(wid, C_DATA, "%s", b->label);
	awprint(wid, C_LABEL, "\"\n");
}

// -----------------------------------------------------------------------
void dbg_c_brk_list(int wid)
{
	struct break_t *b = brk_stack;
	if (!b) {
		awprint(wid, C_LABEL, "No breakpoints\n");
	}
	while (b) {
		if (b->disabled) {
			awprint(wid, C_LABEL, "%i: %s\n", b->nr, b->label);
		} else {
			awprint(wid, C_DATA, "%i: %s\n", b->nr, b->label);
		}
		b = b->next;
	}
}

// -----------------------------------------------------------------------
struct break_t * dbg_c_brk_get(int nr)
{
	struct break_t *b = brk_stack;
	while (b) {
		if (b->nr == nr) {
			return b;
		}
		b = b->next;
	}
	return NULL;
}

// -----------------------------------------------------------------------
void dbg_c_brk_del(int wid, int nr)
{
	struct break_t *b = brk_stack;
	struct break_t *prev = NULL;
	while (b) {
		if (b->nr == nr) {
			if (prev) {
				prev->next = b->next;
			} else {
				brk_stack = brk_last = b->next;
			}
			awprint(wid, C_LABEL, "Removing breakpoint ");
			awprint(wid, C_DATA, "%i", b->nr);
			awprint(wid, C_LABEL, ":");
			awprint(wid, C_DATA, " %s\n", b->label);
			free(b->label);
			n_free_tree(b->n);
			free(b);
			return;
		}
		prev = b;
		b = b->next;
	}
	awprint(wid, C_ERROR, "No such breakpoint: %i\n", nr);
}

// -----------------------------------------------------------------------
void dbg_c_brk_test(int wid, int nr)
{
	struct break_t *b = dbg_c_brk_get(nr);
	if (b) {
		awprint(wid, C_LABEL, "Breakpoint ");
		awprint(wid, C_DATA, "%i", b->nr);
		awprint(wid, C_LABEL, " evaluates to: ");
		awprint(wid, C_DATA, "%i\n", n_eval(b->n));
	} else {
		awprint(wid, C_ERROR, "No such breakpoint: %i\n", nr);
	}
} 

// -----------------------------------------------------------------------
void dbg_c_brk_disable(int wid, int nr, int disable)
{
	struct break_t *b = dbg_c_brk_get(nr);
	if (b) {
		b->disabled = disable;
		if (disable) {
			awprint(wid, C_LABEL, "Breakpoint ");
			awprint(wid, C_DATA, "%i", nr);
			awprint(wid, C_LABEL, " disabled.\n");
		} else {
			awprint(wid, C_LABEL, "Breakpoint ");
			awprint(wid, C_DATA, "%i", nr);
			awprint(wid, C_LABEL, " enabled.\n");
		}
	} else {
		awprint(wid, C_ERROR, "No such breakpoint: %i\n", nr);
	}
}

// vim: tabstop=4
