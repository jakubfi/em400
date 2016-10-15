//  Copyright (c) 2016 Jakub Filipowicz <jakubf@gmail.com>
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

#include <inttypes.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>

#include "ectl_cp.h"

#include "ui/cmd/commands.h"
#include "ui/cmd/utils.h"

struct ui_cmd_command commands[];

// -----------------------------------------------------------------------
void ui_cmd_regs(FILE *out, char *args)
{
	uint16_t regs[ECTL_REG_COUNT];
	ectl_regs_get(regs);

	fprintf(out, "OK: ");
	for (int i=0 ; i<ECTL_REG_COUNT ; i++) {
		fprintf(out, "%s=0x%04x", ectl_reg_name(i), regs[i]);
		if (i<ECTL_REG_COUNT-1) {
			fprintf(out, " ");
		} else {
			fprintf(out, "\n");
		}
	}
}

// -----------------------------------------------------------------------
void ui_cmd_reg(FILE *out, char *args)
{
    char *tok_reg, *remainder;

	ui_cmd_gettok_str(args, &tok_reg, &remainder);
	if (!tok_reg) {
		fprintf(out, "ERR: Missing argument (register_name)\n");
		return;
	}

	int id = ectl_reg_get_id(tok_reg);
	if (id < 0) {
		fprintf(out, "ERR: No such register: %s\n", tok_reg);
		return;
	}

	int r = ectl_reg_get(id);
	fprintf(out, "OK: 0x%04x\n", r);
}

// -----------------------------------------------------------------------
void ui_cmd_regw(FILE *out, char *args)
{
    char *tok_reg, *tok_val, *remainder;

	ui_cmd_gettok_str(args, &tok_reg, &remainder);
	if (!tok_reg) {
		fprintf(out, "ERR: Missing argument (register name)\n");
		return;
	}
	int id = ectl_reg_get_id(tok_reg);
	if (id < 0) {
		fprintf(out, "ERR: No such register: %s\n", tok_reg);
		return;
	}

	int value = ui_cmd_gettok_int(remainder, &tok_val, &remainder);
	if (!tok_val) {
		fprintf(out, "ERR: Missing argument (value)\n");
		return;
	}
	if (value < 0) {
		fprintf(out, "ERR: Value is not a valid 16-bit integer\n");
		return;
	}

	int res = ectl_reg_set(id, value);
	if (res) {
		fprintf(out, "ERR: Wrong register id\n");
		return;
	}

	fprintf(out, "OK: %s=0x%04x\n", tok_reg, (uint16_t) value);
}

// -----------------------------------------------------------------------
void ui_cmd_help(FILE *out, char *args)
{
	char *tok_cmd, *remainder;

	ui_cmd_gettok_str(args, &tok_cmd, &remainder);

	if (!tok_cmd) {
		struct ui_cmd_command *c = commands;
		fprintf(out, "OK: Available commands: ");
		while (c && c->name) {
			fprintf(out, "%s ", c->name);
			c++;
		}
		fprintf(out, "\n");
	} else {
		struct ui_cmd_command *cmd = ui_cmd_find_command(tok_cmd);
		if (cmd) {
			fprintf(out, "OK: %s %s -- %s\n", cmd->name, cmd->args, cmd->desc);
		} else {
			fprintf(out, "ERR: No help for command: %s\n", tok_cmd);
		}
	}
}

// -----------------------------------------------------------------------
void ui_cmd_clock(FILE *out, char *args)
{
	char *tok_state, *remainder;

	int state = ui_cmd_gettok_bool(args, &tok_state, &remainder);

	if (!tok_state) {
		fprintf(out, "OK: CLOCK %s\n", ectl_clock_get() ? "ON" : "OFF");
	} else {
		if (state < 0) {
			fprintf(out, "ERR: Wrong state: %s\n", tok_state);
		} else {
			ectl_clock_set(state);
			fprintf(out, "OK: CLOCK %s\n", ectl_clock_get() ? "ON" : "OFF");
		}
	}
}

// -----------------------------------------------------------------------
void ui_cmd_stop(FILE *out, char *args)
{
	ectl_cpu_stop();
	fprintf(out, "OK: STOP \n");
}

// -----------------------------------------------------------------------
void ui_cmd_start(FILE *out, char *args)
{
	ectl_cpu_start();
	fprintf(out, "OK: START \n");
}

// -----------------------------------------------------------------------
void ui_cmd_cycle(FILE *out, char *args)
{
	ectl_cpu_cycle();
	fprintf(out, "OK: CYCLE\n");
}

// -----------------------------------------------------------------------
void ui_cmd_clear(FILE *out, char *args)
{
	ectl_cpu_clear();
	fprintf(out, "OK: CLEAR\n");
}

// -----------------------------------------------------------------------
void ui_cmd_mem(FILE *out, char *args)
{
	char *tok_seg, *tok_addr, *tok_count, *remainder;

	int seg = ui_cmd_gettok_int(args, &tok_seg, &remainder);
	if (!tok_seg) {
		fprintf(out, "ERR: Missing argument (memory segment)\n");
		return;
	}
	if ((seg < 0) || (seg > 15)) {
		fprintf(out, "ERR: Wrong segment number\n");
		return;
	}

	int addr = ui_cmd_gettok_int(remainder, &tok_addr, &remainder);
	if (!tok_addr) {
		fprintf(out, "ERR: Missing argument (start address)\n");
		return;
	}
	if (addr < 0) {
		fprintf(out, "ERR: Invalid address\n");
		return;
	}

	int count = ui_cmd_gettok_int(remainder, &tok_count, &remainder);
	if (!tok_count) {
		fprintf(out, "ERR: Missing argument (word count)\n");
		return;
	}
	if (count < 1) {
		fprintf(out, "ERR: Invalid word count\n");
		return;
	}

	uint16_t *mbuf = malloc(count * sizeof(uint16_t));
	int processed = 0;

	fprintf(out, "OK:");

	while (processed < count) {
		int words = ectl_mem_get(seg, addr+processed, mbuf, count-processed);
		for (int i=0 ; i<words ; i++) {
			fprintf(out, " 0x%04x", mbuf[i]);
		}
		processed += words;
		if (processed != count) {
			fprintf(out, " ?");
			processed += 0b0001000000000000 - ((addr+processed) & 0b0000111111111111);
		}
	}

	fprintf(out, "\n");

}

// -----------------------------------------------------------------------
void ui_cmd_memw(FILE *out, char *args)
{
	char *tok_seg, *tok_addr, *tok_val, *remainder;

	int seg = ui_cmd_gettok_int(args, &tok_seg, &remainder);
	if (!tok_seg) {
		fprintf(out, "ERR: Missing argument (memory segment)\n");
		return;
	}
	if ((seg < 0) || (seg > 15)) {
		fprintf(out, "ERR: Wrong segment number\n");
		return;
	}

	int addr = ui_cmd_gettok_int(remainder, &tok_addr, &remainder);
	if (!tok_addr) {
		fprintf(out, "ERR: Missing argument (start address)\n");
		return;
	}
	if (addr < 0) {
		fprintf(out, "ERR: Invalid address\n");
		return;
	}

	uint16_t *mbuf = malloc(0x10000 * sizeof(uint16_t));
	int processed = 0;

	do {
		int val = ui_cmd_gettok_int(remainder, &tok_val, &remainder);
		if (tok_val) {
			if (val < 0) {
				fprintf(out, "ERR: Value on position %i is not a valid 16-bit integer\n", processed);
				return;
			}
			mbuf[processed] = val;
			processed++;
		}
	} while (tok_val);

	if (processed < 1) {
		fprintf(out, "ERR: Missing argument (value)\n");
		return;
	}

	if (processed > 65536) {
		fprintf(out, "ERR: Value count exceeds segment size (65536 words)\n");
		return;
	}

	int res = ectl_mem_set(seg, addr, mbuf, processed);
	if (res != processed) {
		fprintf(out, "ERR: Memory write failed at address 0x%04x (%i words written)\n", addr+res, res);
		return;
	}

	fprintf(out, "OK: %i words written\n", res);
}

// -----------------------------------------------------------------------
void ui_cmd_memmap(FILE *out, char *args)
{
	char *tok_seg, *remainder;

	int seg = ui_cmd_gettok_int(args, &tok_seg, &remainder);
	if (!tok_seg) {
		fprintf(out, "ERR: Missing argument (memory segment)\n");
		return;
	}
	if ((seg < 0) || (seg > 15)) {
		fprintf(out, "ERR: Wrong segment number\n");
		return;
	}

	int map = ectl_mem_map(seg);
	fprintf(out, "OK: 0x%04x\n", map);
}

// -----------------------------------------------------------------------
void ui_cmd_int(FILE *out, char *args)
{
	char *tok_int, *remainder;

	int interrupt = ui_cmd_gettok_int(args, &tok_int, &remainder);
	if (!tok_int) {
		fprintf(out, "OK: 0x%08x\n", ectl_int_get());
	} else {
		int res = ectl_int_set(interrupt);
		if (res) {
			fprintf(out, "ERR: Wrong interrupt number: %s\n", tok_int);
		} else {
			fprintf(out, "OK: %i\n", interrupt);
		}
	}
}

// -----------------------------------------------------------------------
void ui_cmd_oprq(FILE *out, char *args)
{
	ectl_oprq();
	fprintf(out, "OK: OPRQ\n");
}

// -----------------------------------------------------------------------
void ui_cmd_state(FILE *out, char *args)
{
	fprintf(out, "OK: 0x%04x\n", ectl_cpu_state_get());
}

// -----------------------------------------------------------------------
void ui_cmd_quit(FILE *out, char *args)
{
	ectl_cpu_quit();
	fprintf(out, "OK: QUIT\n");
}

// -----------------------------------------------------------------------
void ui_cmd_na(FILE *out, char *args)
{
	fprintf(out, "ERR: Command not implemented\n");
}

// -----------------------------------------------------------------------
struct ui_cmd_command commands[] = {
	{ "bin",	"<val>",					"Initiate binary load",		ui_cmd_na },
	{ "brk",	"<expr>",					"Set breakpoint",			ui_cmd_na },
	{ "brkdel",	"<num>",					"Delete breakpoint",		ui_cmd_na },
	{ "brkls",	"[num]",					"Show breakpoints",			ui_cmd_na },
	{ "clear",	"",							"CPU clear (reset)",		ui_cmd_clear },
	{ "clock",	"[on|off]",					"Show/enable/disable clock",ui_cmd_clock },
	{ "cycle",	"",							"Execute one CPU cycle",	ui_cmd_cycle },
	{ "dasm",	"<seg> <addr>",				"Disassemble instruction",	ui_cmd_na },
	{ "eval",	"<expr>",					"Evaluate expression",		ui_cmd_na },
	{ "help",	"",							"Show help",				ui_cmd_help },
	{ "int",	"[interrupt]",				"Show interrupts, send irq",ui_cmd_int },
	{ "load",	"<seg> <addr> <file>",		"Load file to memory",		ui_cmd_na },
	{ "log",	"<on|off>",					"Enable/disable logging",	ui_cmd_na },
	{ "loglvl",	"<component> <level>",		"Set logging level",		ui_cmd_na },
	{ "mem",	"<seg> <addr> [count]",		"Get memory contents",		ui_cmd_mem },
	{ "memfind","<seg> <val>",				"Search memory contents",	ui_cmd_na },
	{ "memmap",	"<seg>",					"Show memory allocation",	ui_cmd_memmap },
	{ "memw",	"<seg> <addr> <val> ...",	"Set memory contents",		ui_cmd_memw },
	{ "oprq",	"",							"Send operator request",	ui_cmd_oprq },
	{ "quit",	"",							"Quit emulation",			ui_cmd_quit },
	{ "reg",	"<name>",					"Show specific register",	ui_cmd_reg },
	{ "regs",	"",							"Show all registers",		ui_cmd_regs },
	{ "regw",	"<name> <value>",			"Set register value",		ui_cmd_regw },
	{ "start",	"",							"Start CPU",				ui_cmd_start },
	{ "state",	"",							"Show CPU state",			ui_cmd_state },
	{ "stop",	"",							"Stop CPU",					ui_cmd_stop },
	{ NULL },
};

// -----------------------------------------------------------------------
struct ui_cmd_command * ui_cmd_find_command(const char *name)
{
	struct ui_cmd_command *c = commands;
	while (name && c && c->name) {
		if (!strcasecmp(name, c->name)) {
			return c;
		}
		c++;
	}

	return NULL;
}

// vim: tabstop=4 shiftwidth=4 autoindent
