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

#define _XOPEN_SOURCE 500

#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "cfg.h"
#include "em400.h"
#include "cpu/cpu.h"
#include "cpu/registers.h"
#include "cpu/interrupts.h"
#include "mem/mem.h"
#include "mem/mem_mega.h"
#include "mem/mem_elwro.h"

#include "errors.h"
#include "utils.h"
#include "log.h"

#include "debugger/awin.h"
#include "debugger/debugger.h"
#include "debugger/cmd.h"
#include "debugger/ui.h"
#include "debugger/parser.h"
#include "debugger/eval.h"
#include "debugger/decode.h"

#include <emcrk/r40.h>

// -----------------------------------------------------------------------
struct cmd_t dbg_commands[] = {
	{ "quit",	F_QUIT,		"Quit the emulator", "  quit" },
	{ "step",	F_STEP,		"Execute instruction at IC", "  step" },
	{ "help",	F_HELP,		"Print help", "  help" },
	{ "regs",	F_REGS,		"Show user registers", "  regs" },
	{ "sregs",	F_SREGS,	"Show system registers", "  sregs" },
	{ "reset",	F_RESET,	"Reset the emulator", "  reset" },
	{ "dasm",	F_DASM,		"Disassembler", "  dasm [[start] count]" },
	{ "mem",	F_MEM,		"Show memory contents", "  mem [block:] <start> [len]" },
	{ "load",	F_LOAD,		"Load memory image from file", "  load <file>" },
	{ "memcfg",	F_MEMCFG,	"Show memory configuration", "  memcfg" },
	{ "brk",	F_BRK,		"Manipulate breakpoints", "  brk add <expression>\n  brk del <brk_number>\n  brk" },
	{ "run",	F_RUN,		"Run emulation", "  run" },
	{ "stk",	F_STACK,	"Show stack", "  stk" },
	{ "log",	F_LOG,		"Manipulate logging", "  log\n  log on|off\n  log level <component>:<level>" },
	{ "watch",	F_WATCH,	"Manipulate expression watches", "  watch add <expression>\n  watch del <watch_number>\n  watch" },
	{ "decode",	F_DECODE,	"Decode memory structures", "  decode\n  decode <decoder> <address>" },
	{ "find",	F_FIND,		"Search memory for a value", "  find <block> <value>" },
	{ NULL,		0,			NULL, NULL }
};

// -----------------------------------------------------------------------
void dbg_c_load(int wid, char* image)
{
	int res = mem_load(image, 0, 0, 0);
	if (res < E_OK) {
		awtbprint(wid, C_ERROR, "Error loading image \"%s\": %s\n", image, get_error(res));
	}
}

// -----------------------------------------------------------------------
void dbg_c_help(int wid, char *cmd)
{
	struct cmd_t *c = dbg_commands;
	if (cmd) {
		while (c->cmd) {
			if (!strcmp(cmd, c->cmd)) {
				awtbprint(wid, C_DATA, "%s ", c->cmd);
				awtbprint(wid, C_LABEL, ": %s\n", c->doc);
				awtbprint(wid, C_LABEL, "Usage:\n%s\n", c->help);
				return;
			}
			c++;
		}
		awtbprint(wid, C_ERROR, "Error: no such command: %s\n", cmd);
	} else {
		while (c->cmd) {
			awtbprint(wid, C_LABEL, "%-10s : %s\n", c->cmd, c->doc);
			c++;
		}
	}
}

// -----------------------------------------------------------------------
void dbg_c_quit()
{
	cpu_state = STATE_QUIT;
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
	cpu_reset(1);
}

// -----------------------------------------------------------------------
void dbg_c_dt(int wid, uint16_t ic, int count)
{
	int words;
	char *buf = emdas_get_buf(emd);

	while (count > 0) {
		words = emdas_dasm(emd, QNB, ic);
		emdas_get_buf(emd);

		if (ic == regs[R_IC]) {
			if (P) {
				awtbprint(wid, C_IRED, "0x%04x", ic);
				awtbprint(wid, C_IRED, " %-20s", buf);
			} else {
				awtbprint(wid, C_ILABEL, "0x%04x", ic);
				awtbprint(wid, C_IDATA, " %-20s", buf);
			}
		} else {
			awtbprint(wid, C_LABEL, "0x%04x", ic);
			if (*buf == '.') {
				awtbprint(wid, C_DATA, " %-20s", buf);
			} else if (*buf == '?') {
				awtbprint(wid, C_ERROR, " %-20s", buf);
			} else {
				awtbprint(wid, C_PROMPT, " %-20s", buf);
			}
		}

		awtbprint(wid, C_DATA, "\n");
		count--;
		ic += words;
	}
}

// -----------------------------------------------------------------------
void dbg_c_mem(int wid, int block, int start, int end, int maxcols, int maxlines)
{
	uint16_t data;
	int res;

	// wrong range
	if ((end - start) <= 0) {
		awtbprint(wid, C_ERROR, "Wrong memory range: %i - %i\n", start, end);
		return;
	}

	int words = (maxcols - 10) / 7;
	if (words>16) words = 16;
	maxlines -=2; // two used for header

	// headers
	awtbprint(wid, C_LABEL, "  addr: ");
	for (int i=0 ; i<words ; i++) awtbprint(wid, C_LABEL, "+%03x ", i);
	awtbprint(wid, C_LABEL, "\n");
	awtbprint(wid, C_LABEL, "-------");
	for (int i=0 ; i<words ; i++) awtbprint(wid, C_LABEL, "-----");
	awtbprint(wid, C_LABEL, "  ");
	for (int i=0 ; i<words ; i++) awtbprint(wid, C_LABEL, "--");
	awtbprint(wid, C_LABEL, "\n");

	uint16_t addr = start;
	int attr = C_DATA;

	while ((maxlines > 0) && (addr <= end)) {
		// row header
		awtbprint(wid, C_LABEL, "0x%04x: ", addr);
		char *chars = malloc(words*2+1);
		for (int w=0 ; w<words ; w++) {
			res = mem_get(block, addr, &data);
			if (!res) {
				awtbprint(wid, C_ERROR, "~~~~ ");
				chars[w*2] = '~';
				chars[w*2+1] = '~';
			} else {
				// cell with current instruction
				if (addr == regs[R_IC]) {
					attr = C_DATAU;
				} else {
					attr = C_DATA;
				}

				awtbprint(wid, attr, "%4x", data);
				awtbprint(wid, C_DATA, " ");

				// store data (chars)
				int2chars(data, chars+w*2);
			}
			addr++;
		}
		// data (chars)
		awtbprint(wid, C_DATA, " %s\n", chars);
		free(chars);
		maxlines--;
	}
}

// -----------------------------------------------------------------------
void dbg_c_sregs(int wid)
{
	awtbprint(wid, C_LABEL, "            OPCODE D A   B   C");
	awtbprint(wid, C_LABEL, "           P: ");
	awtbprint(wid, C_DATA, "%i\n", P);
	awtbprint(wid, C_LABEL, "IR: ");
	awtbprint(wid, C_DATA, "0x%04x  ", regs[R_IR]);
	awtbbinprint(wid, C_DATA, "...... . ... ... ...", regs[R_IR], 16);
	awtbprint(wid, C_LABEL, "        IC: ");
	awtbprint(wid, C_DATA, "0x%04x ", regs[R_IC]);
	awtbprint(wid, C_DATA, "\n");
	awtbprint(wid, C_LABEL, "            PMCZs139fS Q s NB");
	awtbprint(wid, C_LABEL, "          MOD: ");
	awtbprint(wid, C_DATA, "0x%04x (%i)", regs[R_MOD], regs[R_MODc]);
	awtbprint(wid, C_DATA, "\n");
	awtbprint(wid, C_LABEL, "SR: ");
	awtbprint(wid, C_DATA, "0x%04x  ", regs[R_SR]);
	awtbbinprint(wid, C_DATA, ".......... . . ....", regs[R_SR], 16);
	awtbprint(wid, C_DATA, "\n");

	awtbprint(wid, C_LABEL, "KB: ");
	awtbprint(wid, C_DATA, "0x%04x  ", regs[R_KB]);
	awtbbinprint(wid, C_DATA, "........ ........", regs[R_KB], 16);
	awtbprint(wid, C_DATA, "\n\n");

	awtbprint(wid, C_LABEL, "                ZPMCZ TIFFFFx 01 23 456789 abcdef OCSS");
	awtbprint(wid, C_DATA, "\n");
	awtbprint(wid, C_LABEL, "RZ: ");
	awtbprint(wid, C_DATA, "0x%08x  ", RZ);
	awtbbinprint(wid, C_DATA, "..... ....... .. .. ...... ...... ....", RZ, 32);
	awtbprint(wid, C_DATA, "\n");
}

// -----------------------------------------------------------------------
void dbg_c_regs(int wid)
{
	awtbprint(wid, C_LABEL, "    hex    oct    dec    ZMVCLEGY Xuser    ch R40\n");
	for (int i=0 ; i<=7 ; i++) {
		char *r = r40_to_ascii(regs+i, 1, NULL);
		char c[3];
		int2chars(regs[i], c);

		awtbprint(wid, C_LABEL, "R%i: ", i);
		awtbprint(wid, C_DATA, "0x%04x %6o %6i ", regs[i], regs[i], (int16_t) regs[i]);
		awtbbinprint(wid, C_DATA, "........ ........", regs[i], 16);
		awtbprint(wid, C_DATA, " %-3s %-3s\n", c, r);
		free(r);
	}
}

// -----------------------------------------------------------------------
void dbg_c_stack(int wid, int size)
{
	static int sb;
	static int osp;
	uint16_t data;
	int res;

	res = mem_get(0, 97, &data);
	if (!res) {
		awtbprint(wid, C_ERROR, "~~~~");
		return;
	}

	int sp = data;

	if ((sb <= 0) || (sp-osp > 4) || (sp-osp < -4)) {
		sb = sp;
	}

	osp = sp;
	res = mem_get(0, sp, &data);

	while ((sp >= sb) && res) {
		if (sp == osp) {
			awtbprint(wid, C_ILABEL, " 0x%04x: ", sp);
			awtbprint(wid, C_IDATA, "%04x \n", data);
		} else {
			awtbprint(wid, C_LABEL, " 0x%04x: ", sp);
			awtbprint(wid, C_DATA, "%04x \n", data);
		}
		sp--;
		res = mem_get(0, sp, &data);
	}
}

// -----------------------------------------------------------------------
char dbg_mem_getalloc(int mod, int seg)
{
	int i, j;

	if (!mem_map[mod][seg].seg) {
		return '.';
	} else if (mem_map[mod][seg].seg == mem_mega_prom) {
		return 'p';
	} else {
		for (i=0 ; i<MEM_MAX_MODULES ; i++) {
			for (j=0 ; j<MEM_MAX_MEGA_SEGMENTS ; j++) {
				if ((j<MEM_MAX_ELWRO_SEGMENTS) && (mem_map[mod][seg].seg == mem_elwro[i][j])) {
					return 'e';
				} else if (mem_map[mod][seg].seg == mem_mega[i][j]) {
					return 'm';
				}
			}
		}
	}

	return '?';
}

// -----------------------------------------------------------------------
void dbg_c_memcfg(int wid)
{
	int i, j, cnt;
	char c;
	awtbprint(wid, C_LABEL, "module  :  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15\n");

	awtbprint(wid, C_LABEL, "Elwro   : ");
	for (i=0 ; i<MEM_MAX_MODULES ; i++) {
		cnt = 0;
		for (j=0 ; j<MEM_MAX_ELWRO_SEGMENTS ; j++) {
			if (mem_elwro[i][j]) cnt++;
		}
		awtbprint(wid, C_DATA, "%2i ", cnt);
	}
	awtbprint(wid, C_DATA, "\n");

	awtbprint(wid, C_LABEL, "MEGA    : ");
	for (i=0 ; i<MEM_MAX_MODULES ; i++) {
		cnt = 0;
		for (j=0 ; j<MEM_MAX_MEGA_SEGMENTS ; j++) {
			if (mem_mega[i][j]) cnt++;
		}
		awtbprint(wid, C_DATA, "%2i ", cnt);
	}
	awtbprint(wid, C_DATA, "\n");

	awtbprint(wid, C_LABEL, "\nAllocation map:\n");
	awtbprint(wid, C_LABEL, "    0 1 2 3 4 5 6 7 8 9 a b c d e f\n");
	for (i=0 ; i<MEM_MAX_MODULES ; i++) {
		cnt = 0;
		awtbprint(wid, C_LABEL, "%2i: ", i);
		for (j=0 ; j<MEM_MAX_MEGA_SEGMENTS ; j++) {
			c = dbg_mem_getalloc(i, j);
			if (c != '.') {
				cnt++;
			}
			awtbprint(wid, C_DATA, "%c ", c);
		}
		awtbprint(wid, C_LABEL, " = %3i K\n", cnt*4);
	}
	awtbprint(wid, C_DATA, "\n");
}

// -----------------------------------------------------------------------
void dbg_c_brk_add(int wid, char *label, struct node_t *n)
{
	static int brkcnt;

	struct evlb_t *b = malloc(sizeof(struct evlb_t));
	b->nr = brkcnt++;
	b->value = 0;
	b->disabled = 0;
	b->label = strdup(label);
	b->n = n;
	b->next = NULL;

	if (brk_top) {
		brk_top->next = b;
		brk_top = b;
	} else {
		brk_top = brk_stack = b;
	}

	awtbprint(wid, C_LABEL, "Breakpoint ");
	awtbprint(wid, C_DATA, "%i", b->nr);
	awtbprint(wid, C_LABEL, " added: \"");
	awtbprint(wid, C_DATA, "%s", b->label);
	awtbprint(wid, C_LABEL, "\"\n");
}

// -----------------------------------------------------------------------
void dbg_c_brk_list(int wid)
{
	struct evlb_t *b = brk_stack;
	if (!b) {
		awtbprint(wid, C_LABEL, "No breakpoints\n");
	}
	while (b) {
		if (b->disabled) {
			awtbprint(wid, C_LABEL, "%i: %s (disabled)\n", b->nr, b->label);
		} else {
			awtbprint(wid, C_DATA, "%i: %s\n", b->nr, b->label);
		}
		b = b->next;
	}
}

// -----------------------------------------------------------------------
struct evlb_t * dbg_c_brk_get(int nr)
{
	struct evlb_t *b = brk_stack;
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
	struct evlb_t *b = brk_stack;
	struct evlb_t *prev = NULL;
	while (b) {
		if (b->nr == nr) {
			if (prev) {
				prev->next = b->next;
			} else {
				brk_stack = brk_top = b->next;
			}
			awtbprint(wid, C_LABEL, "Removing breakpoint ");
			awtbprint(wid, C_DATA, "%i", b->nr);
			awtbprint(wid, C_LABEL, ":");
			awtbprint(wid, C_DATA, " %s\n", b->label);
			free(b->label);
			n_free_tree(b->n);
			free(b);
			return;
		}
		prev = b;
		b = b->next;
	}
	awtbprint(wid, C_ERROR, "No such breakpoint: %i\n", nr);
}

// -----------------------------------------------------------------------
void dbg_c_brk_test(int wid, int nr)
{
	struct evlb_t *b = dbg_c_brk_get(nr);
	if (b) {
		awtbprint(wid, C_LABEL, "Breakpoint ");
		awtbprint(wid, C_DATA, "%i", b->nr);
		awtbprint(wid, C_LABEL, " evaluates to: ");
		awtbprint(wid, C_DATA, "%i\n", n_eval(b->n));
	} else {
		awtbprint(wid, C_ERROR, "No such breakpoint: %i\n", nr);
	}
}

// -----------------------------------------------------------------------
void dbg_c_brk_disable(int wid, int nr, int disable)
{
	struct evlb_t *b = dbg_c_brk_get(nr);
	if (b) {
		b->disabled = disable;
		if (disable) {
			awtbprint(wid, C_LABEL, "Breakpoint ");
			awtbprint(wid, C_DATA, "%i", nr);
			awtbprint(wid, C_LABEL, " disabled.\n");
		} else {
			awtbprint(wid, C_LABEL, "Breakpoint ");
			awtbprint(wid, C_DATA, "%i", nr);
			awtbprint(wid, C_LABEL, " enabled.\n");
		}
	} else {
		awtbprint(wid, C_ERROR, "No such breakpoint: %i\n", nr);
	}
}

// -----------------------------------------------------------------------
void dbg_c_brk_disable_all(int wid, int disable)
{
	struct evlb_t *b = brk_stack;
	while (b) {
		dbg_c_brk_disable(wid, b->nr, disable);
		b = b->next;
	}
}

// -----------------------------------------------------------------------
void dbg_c_log_info(int wid)
{
	int i;
	char *cname;

	awtbprint(wid, C_DATA, "Logging %s\n", log_is_enabled() ? "enabled" : "disabled");
	awtbprint(wid, C_LABEL, "Levels: ");

	for (i=0 ; i < L_ALL ; i++) {
		cname = log_get_component_name(i);
		if (cname) {
			awtbprint(wid, C_DATA, "%s=%i ", cname, log_get_level(i));
		}
	}
	awtbprint(wid, C_LABEL, "\n");
}

// -----------------------------------------------------------------------
void dbg_c_log_disable(int wid)
{
	log_disable();
	awtbprint(wid, C_LABEL, "Logging disabled\n");
}

// -----------------------------------------------------------------------
void dbg_c_log_enable(int wid)
{
	int res = log_enable();
	if (res == E_OK) {
		awtbprint(wid, C_LABEL, "Logging enabled\n");
	} else {
		awtbprint(wid, C_ERROR, "Error opening log file\n");
	}
}

// -----------------------------------------------------------------------
void dbg_c_log_set_level(int wid, char *comp_name, int level)
{
	int c;

	c = log_get_component_id(comp_name);
	if (c < 0) {
		awtbprint(wid, C_ERROR, "Unknown component: ");
		awtbprint(wid, C_DATA, "%s", comp_name);
	} else {
		log_set_level(c, level);
		awtbprint(wid, C_LABEL, "Level for component ");
		awtbprint(wid, C_DATA, "%s ", comp_name);
		awtbprint(wid, C_LABEL, "set to ");
		awtbprint(wid, C_DATA, "%i\n", level);
	}
}

// -----------------------------------------------------------------------
void dbg_c_watch_list(int wid, int count)
{
	struct evlb_t *w = watch_stack;
	if (!w) {
		awtbprint(wid, C_LABEL, "No watches\n");
	}

	while (w && (count > 0)) {
		int value = n_eval(w->n);
		awtbprint(wid, C_LABEL, "%i: ", w->nr);
		awtbprint(wid, C_DATA, "%-6s ", w->label);
		awtbprint(wid, C_LABEL, "= ");
		awtbprint(wid, C_DATA, "0x%04x (%i)\n", (uint16_t) value, (int16_t) value);
		w = w->next;
		count--;
	}
}

// -----------------------------------------------------------------------
void dbg_c_watch_add(int wid, char *label, struct node_t *n)
{
	static int watchcnt;
	struct evlb_t *w = malloc(sizeof(struct evlb_t));
	w->nr = watchcnt++;
	w->value = 0;
	w->disabled = 0;
	w->label = strdup(label);
	w->n = n;
	w->next = NULL;

	if (watch_top) {
		watch_top->next = w;
		watch_top = w;
	} else {
		watch_top = watch_stack = w;
	}

	awtbprint(wid, C_LABEL, "Watch ");
	awtbprint(wid, C_DATA, "%i", w->nr);
	awtbprint(wid, C_LABEL, " added: \"");
	awtbprint(wid, C_DATA, "%s", w->label);
	awtbprint(wid, C_LABEL, "\"\n");

}

// -----------------------------------------------------------------------
void dbg_c_watch_del(int wid, int nr)
{
	struct evlb_t *w = watch_stack;
	struct evlb_t *prev = NULL;

	while (w) {
		if (w->nr == nr) {
			if (prev) {
				prev->next = w->next;
			} else {
				watch_stack = watch_top = w->next;
			}
			awtbprint(wid, C_LABEL, "Removing watch ");
			awtbprint(wid, C_DATA, "%i", w->nr);
			awtbprint(wid, C_LABEL, ":");
			awtbprint(wid, C_DATA, " %s\n", w->label);
			free(w->label);
			n_free_tree(w->n);
			free(w);
			return;
		}
		prev = w;
		w = w->next;
	}
	awtbprint(wid, C_ERROR, "No such watch: %i\n", nr);
}

// -----------------------------------------------------------------------
void dbg_c_list_decoders(int wid)
{
	awtbprint(wid, C_DATA, "Available decoders:\n");
	struct decoder_t *d = decoders;
	while (d && d->name) {
		awtbprint(wid, C_DATA, "  %-8s", d->name);
		awtbprint(wid, C_LABEL, " %s\n", d->desc);
		d++;
	}
}

// -----------------------------------------------------------------------
void dbg_c_decode(int wid, char *name, uint16_t addr, int arg)
{
	struct decoder_t *d = find_decoder(name);
	char *buf = NULL;

	if (!d) {
		awtbprint(wid, C_ERROR, "No such decoder: %s\n", name);
		return;
	}

	buf = d->f_decode(NB, addr, arg);

	if (!buf) {
		awtbprint(wid, C_ERROR, "Cannot decode structure\n");
		return;
	}

	awtbprint(wid, C_DATA, "Decoding structure at 0x%04x as: %s\n", addr, d->desc);
	awtbprint(wid, C_LABEL, "-----------------------------------------------------------\n");
	awtbprint(wid, C_LABEL, buf);
	free(buf);
	awtbprint(wid, C_LABEL, "-----------------------------------------------------------\n");
}

// -----------------------------------------------------------------------
void dbg_c_find(int wid, uint16_t block, uint16_t value)
{
	int found = 0;
	for (int seg=0 ; seg<MEM_MAX_SEGMENTS ; seg++) {
		if (mem_map[block][seg].seg) {
			awtbprint(wid, C_LABEL, "Segment %x: ", seg);
			for (int word=0 ; word<MEM_SEGMENT_SIZE ; word++) {
				if (mem_map[block][seg].seg[word] == value) {
					awtbprint(wid, C_DATA, "0x%04x ", (seg<<12)+word);
					found++;
					if (found >= 8) {
						awtbprint(wid, C_DATA, "\n           ");
						found = 0;
					}
				}
			}
			if (found < 8) {
				awtbprint(wid, C_DATA, "\n");
			}
		}
	}
}

// vim: tabstop=4 shiftwidth=4 autoindent
