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

#include <stdlib.h>
#include <string.h>
#include <strings.h>

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
#include "debugger/log.h"

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
	{ "memcl",	F_MEMCL,	"Clear memory contents", "  memcl" },
	{ "load",	F_LOAD,		"Load memory image from file", "  load <file> [mem_block]" },
	{ "memcfg",	F_MEMCFG,	"Show memory configuration", "  memcfg" },
	{ "brk",	F_BRK,		"Manipulate breakpoints", "  brk add <expression>\n  brk list\n  brk del <brk_number>" },
	{ "run",	F_RUN,		"Run emulation", "  run" },
	{ "stack",	F_STACK,	"Show stack", "  stack" },
	{ "log",	F_LOG,		"Enable logging", "  log\n  log on|off\n  log file <filename>\n  log level <domain>:<level>" },
	{ NULL,		0,			NULL }
};

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
	dbg_touch_drop_all(&touch_mem);
	dbg_touch_drop_all(&touch_reg);
}

// -----------------------------------------------------------------------
void dbg_c_clmem()
{
	mem_clear();
}

// -----------------------------------------------------------------------
void dbg_c_dt(int wid, int dasm_mode, uint16_t start, int count)
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
				if (*buf != '-') awprint(wid, C_PROMPT, " %-19s\n", buf);
				else awprint(wid, C_DATA, " %-19s\n", buf);
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

			// cell with current instruction
			if (addr == regs[R_IC]) {
				attr = C_DATAU;
			} else {
				attr = dbg_touch2attr(dbg_touch_check(&touch_mem, block, addr));
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
	awprint(wid, C_LABEL, "            OPCODE D A   B   C");
	awprint(wid, C_LABEL, "           P: ");
	awprint(wid, C_DATA, "\n");
	awprint(wid, C_LABEL, "IR: ");
	awprint(wid, C_DATA, "0x%04x  ", regs[R_IR]);
	awbinprint(wid, C_DATA, "...... . ... ... ...", regs[R_IR], 16);
	awprint(wid, C_LABEL, "        IC: ");
	awprint(wid, C_DATA, "0x%04x ", regs[R_IC]);
	awprint(wid, C_DATA, "\n");
	awprint(wid, C_LABEL, "            PMCZs139fS Q s NB");
	awprint(wid, C_LABEL, "          MOD: ");
	awprint(wid, C_DATA, "0x%04x (%i)", regs[R_MOD], regs[R_MODc]);
	awprint(wid, C_DATA, "\n");
	awprint(wid, C_LABEL, "SR: ");
	awprint(wid, C_DATA, "0x%04x  ", regs[R_SR]);
	awbinprint(wid, C_DATA, ".......... . . ....", regs[R_SR], 16);
	awprint(wid, C_LABEL, "       ZC17: ");
	awprint(wid, C_DATA, "%i", regs[R_ZC17]);
	awprint(wid, C_DATA, "\n");

	awprint(wid, C_LABEL, "KB: ");
	awprint(wid, C_DATA, "0x%04x  ", regs[R_KB]);
	awbinprint(wid, C_DATA, "........ ........", regs[R_KB], 16);
	awprint(wid, C_DATA, "\n");

	awprint(wid, C_LABEL, "                ZPMCZ TIFFFFx 01 23 456789 abcdef OCSS");
	awprint(wid, C_DATA, "\n");
	awprint(wid, C_LABEL, "RZ: ");
	awprint(wid, C_DATA, "0x%08x  ");
	awbinprint(wid, C_DATA, "..... ....... .. .. ...... ...... ....", RZ, 32);
	awprint(wid, C_DATA, "\n");

	awprint(wid, C_LABEL, "RP: ");
	awprint(wid, C_DATA, "0x%08x  ");
	awbinprint(wid, C_DATA, "..... ....... .. .. ...... ...... ....", RP, 32);
	awprint(wid, C_DATA, "\n");

	uint32_t int_act = 0;
	struct touch_t *t = touch_int;
	while (t) {
		int_act |= 1 << (31-t->pos);
		t = t->next;
	}

	awprint(wid, C_LABEL, "Being served:   ");
	awbinprint(wid, C_LABEL, "..... ....... .. .. ...... ...... ....", int_act, 32);
	awprint(wid, C_LABEL, "\n");
}

// -----------------------------------------------------------------------
void dbg_c_regs(int wid)
{
	awprint(wid, C_LABEL, "    hex    oct    dec    ZMVCLEGY Xuser    ch R40\n");
	for (int i=0 ; i<=7 ; i++) {
		char *r = int2r40(regs[i]);
		char c[3];
		int2chars(regs[i], c);

		int attr = dbg_touch2attr(dbg_touch_check(&touch_reg, 0, i));

		awprint(wid, C_LABEL, "R%i: ", i);
		awprint(wid, attr, "0x%04x %6o %6i ", regs[i], regs[i], (int16_t) regs[i]);
		awbinprint(wid, attr, "........ ........", regs[i], 16);
		awprint(wid, attr, " %s %s\n", c, r);
		free(r);
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
	awprint(wid, C_LABEL, "Number of 4kword pages in each hardware module and logical block\n");
	awprint(wid, C_LABEL, "mod/blk:   0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15\n");
	awprint(wid, C_LABEL, " hw pgs:  ");
	for (i=0 ; i<MEM_MAX_MODULES ; i++) {
		cnt = 0;
		for (j=0 ; j<MEM_MAX_SEGMENTS ; j++) {
			if (mem_segment[i][j]) cnt++;
		}
		awprint(wid, C_DATA, "%2i ", cnt);
	}
	awprint(wid, C_DATA, "\n");
	awprint(wid, C_LABEL, " sw pgs:  ");
	for (i=0 ; i<MEM_MAX_NB ; i++) {
		cnt = 0;
		for (j=0 ; j<MEM_MAX_SEGMENTS ; j++) {
			if (mem_map[i][j]) cnt++;
		}
		awprint(wid, C_DATA, "%2i ", cnt);
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
			awprint(wid, C_LABEL, "%i: %s (disabled)\n", b->nr, b->label);
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

// -----------------------------------------------------------------------
void dbg_c_log_show(int wid)
{
	awprint(wid, C_LABEL, "Logging to file: ");
	awprint(wid, C_DATA, "%s (%s)\n", log_fname, log_enabled ? "enabled" : "disabled");
	awprint(wid, C_LABEL, "Levels: ");
	char **d = log_dname;
	int i = 0;
	while (*(d+i)) {
		awprint(wid, C_DATA, "%s:%i ", *(d+i), log_level[i]);
		i++;
	}
	awprint(wid, C_LABEL, "\n");
}

// -----------------------------------------------------------------------
void dbg_c_log_set(int wid, char *domain, int level)
{
	int d = log_find_domain(domain);
	if (d < -1) {
		awprint(W_CMD, C_ERROR, "Unknown domain: %s\n", domain);
	} else {
		log_setlevel(d, level);
	}
}

// vim: tabstop=4
