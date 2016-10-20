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
#include <stdarg.h>

#include "ectl.h"

#include "ui/cmd/commands.h"
#include "ui/cmd/utils.h"

struct ui_cmd_command commands[];

// -----------------------------------------------------------------------
void ui_cmd_resp(FILE *out, int status, int eol, char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	if (status == RESP_OK) {
		fprintf(out, "OK:");
	} else {
		fprintf(out, "ERR:");
	}
	if (eol) {
		fprintf(out, " ");
	}
	vfprintf(out, fmt, ap);
	va_end(ap);
	if (eol) {
		fprintf(out, "\n");
	}
}

// -----------------------------------------------------------------------
void ui_cmd_reg(FILE *out, char *args)
{
	char *tok_reg, *tok_val, *remainder;

	ui_cmd_gettok_str(args, &tok_reg, &remainder);

	// show all registers
	if (!tok_reg) {
		uint16_t regs[ECTL_REG_COUNT];
		ectl_regs_get(regs);

		ui_cmd_resp(out, RESP_OK, UI_NOEOL, "");
		for (int i=0 ; i<ECTL_REG_COUNT ; i++) {
			fprintf(out, " %s=0x%04x", ectl_reg_name(i), regs[i]);
		}
		fprintf(out, "\n");
		return;
	}

	// find register
	int id = ectl_reg_get_id(tok_reg);
	if (id < 0) {
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "No such register: %s", tok_reg);
		return;
	}

	int value = ui_cmd_gettok_int(remainder, &tok_val, &remainder);

	// show specific register
	if (!tok_val) {
		ui_cmd_resp(out, RESP_OK, UI_EOL, "%s=0x%04x", ectl_reg_name(id), ectl_reg_get(id));
		return;
	}

	// check value
	if (value < 0) {
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "Value is not a valid 16-bit integer");
		return;
	}

	int res = ectl_reg_set(id, value);
	if (res) {
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "Wrong register id");
		return;
	}

	ui_cmd_resp(out, RESP_OK, UI_EOL, "%s=0x%04x", ectl_reg_name(id), ectl_reg_get(id));
}

// -----------------------------------------------------------------------
void ui_cmd_help(FILE *out, char *args)
{
	char *tok_cmd, *remainder;

	ui_cmd_gettok_str(args, &tok_cmd, &remainder);

	// show all commands
	if (!tok_cmd) {
		struct ui_cmd_command *c = commands;
		ui_cmd_resp(out, RESP_OK, UI_NOEOL, " Available commands:");
		while (c && c->name) {
			fprintf(out, " %s", c->name);
			c++;
		}
		fprintf(out, "\n");
		return;
	}

	struct ui_cmd_command *cmd = ui_cmd_find_command(tok_cmd);

	// no such command
	if (!cmd) {
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "No help for command: %s", tok_cmd);
	}

	// show specific command help
	ui_cmd_resp(out, RESP_OK, UI_EOL, "%s %s : %s", cmd->name, cmd->args, cmd->desc);
}
// -----------------------------------------------------------------------
void ui_cmd_info(FILE *out, char *args)
{
	int capa = ectl_capa();
	ui_cmd_resp(out, RESP_OK, UI_NOEOL, " EM400 %s", ectl_version());
	for (int i=0 ; i<ECTL_CAPA_COUNT ; i++) {
		if ((capa & (1<<i))) {
			fprintf(out, " %s", ectl_capa_name(i));
		}
	}
	fprintf(out, "\n");
}

// -----------------------------------------------------------------------
void ui_cmd_clock(FILE *out, char *args)
{
	char *tok_state, *remainder;

	int state = ui_cmd_gettok_bool(args, &tok_state, &remainder);

	// show clock state
	if (!tok_state) {
		ui_cmd_resp(out, RESP_OK, UI_EOL, "CLOCK %s", ectl_clock_get() ? "ON" : "OFF");
		return;
	}

	// is state correct?
	if (state < 0) {
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "Wrong state: %s", tok_state);
		return;
	}

	// set clock state
	ectl_clock_set(state);
	ui_cmd_resp(out, RESP_OK, UI_EOL, "CLOCK %s", ectl_clock_get() ? "ON" : "OFF");
}

// -----------------------------------------------------------------------
void ui_cmd_stop(FILE *out, char *args)
{
	ectl_cpu_stop();
	ui_cmd_resp(out, RESP_OK, UI_EOL, "STOP");
}

// -----------------------------------------------------------------------
void ui_cmd_start(FILE *out, char *args)
{
	ectl_cpu_start();
	ui_cmd_resp(out, RESP_OK, UI_EOL, "START");
}

// -----------------------------------------------------------------------
void ui_cmd_cycle(FILE *out, char *args)
{
	ectl_cpu_cycle();
	ui_cmd_resp(out, RESP_OK, UI_EOL, "CYCLE");
}

// -----------------------------------------------------------------------
void ui_cmd_clear(FILE *out, char *args)
{
	ectl_cpu_clear();
	ui_cmd_resp(out, RESP_OK, UI_EOL, "CLEAR");
}

// -----------------------------------------------------------------------
void ui_cmd_mem(FILE *out, char *args)
{
	char *tok_seg, *tok_addr, *tok_count, *remainder;

	int seg = ui_cmd_gettok_int(args, &tok_seg, &remainder);
	if (!tok_seg) {
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "Missing argument (memory segment)");
		return;
	}
	if ((seg < 0) || (seg > 15)) {
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "Wrong segment number");
		return;
	}

	int addr = ui_cmd_gettok_int(remainder, &tok_addr, &remainder);
	if (!tok_addr) {
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "Missing argument (start address)");
		return;
	}
	if (addr < 0) {
		ui_cmd_resp(out, RESP_ERR, UI_EOL,  "Invalid address");
		return;
	}

	int count = ui_cmd_gettok_int(remainder, &tok_count, &remainder);
	if (!tok_count) {
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "Missing argument (word count)");
		return;
	}
	if (count < 1) {
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "Invalid word count");
		return;
	}

	uint16_t *mbuf = malloc(count * sizeof(uint16_t));
	int processed = 0;

	ui_cmd_resp(out, RESP_OK, UI_NOEOL, "");

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
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "Missing argument (memory segment)");
		return;
	}
	if ((seg < 0) || (seg > 15)) {
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "Wrong segment number");
		return;
	}

	int addr = ui_cmd_gettok_int(remainder, &tok_addr, &remainder);
	if (!tok_addr) {
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "Missing argument (start address)");
		return;
	}
	if (addr < 0) {
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "Invalid address");
		return;
	}

	uint16_t *mbuf = malloc(0x10000 * sizeof(uint16_t));
	int processed = 0;

	do {
		int val = ui_cmd_gettok_int(remainder, &tok_val, &remainder);
		if (tok_val) {
			if (val < 0) {
				ui_cmd_resp(out, RESP_ERR, UI_EOL, "Value on position %i is not a valid 16-bit integer", processed);
				return;
			}
			mbuf[processed] = val;
			processed++;
		}
	} while (tok_val);

	if (processed < 1) {
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "Missing argument (value)");
		return;
	}

	if (processed > 65536) {
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "Value count exceeds segment size (65536 words)");
		return;
	}

	int res = ectl_mem_set(seg, addr, mbuf, processed);
	if (res != processed) {
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "Memory write failed at address 0x%04x (%i words written)", addr+res, res);
		return;
	}

	ui_cmd_resp(out, RESP_OK, UI_EOL, "%i words written", res);
}

// -----------------------------------------------------------------------
void ui_cmd_load(FILE *out, char *args)
{
	char *tok_seg, *tok_addr, *tok_file, *remainder;

	int seg = ui_cmd_gettok_int(args, &tok_seg, &remainder);
	if (!tok_seg) {
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "Missing argument (memory segment)");
		return;
	}
	if ((seg < 0) || (seg > 15)) {
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "Wrong segment number");
		return;
	}

	int addr = ui_cmd_gettok_int(remainder, &tok_addr, &remainder);
	if (!tok_addr) {
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "Missing argument (start address)");
		return;
	}
	if (addr < 0) {
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "Invalid address");
		return;
	}

	tok_file = ui_cmd_skip_ws(remainder);
	if (!tok_file || !*tok_file) {
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "Missing argument (file name)");
		return;
	}
	tok_file = ui_cmd_remove_trailing_ws(tok_file);

	FILE *f = fopen(tok_file, "rb");
	if (!f) {
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "Error opening file: '%s'", tok_file);
		return;
	}

	int res = ectl_load(f, seg, addr);
	if (res < 0) {
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "Error reading file: '%s'", tok_file);
	} else {
		ui_cmd_resp(out, RESP_OK, UI_EOL, "%i words loaded from file '%s'", res, tok_file);
	}

	fclose(f);
}

// -----------------------------------------------------------------------
void ui_cmd_memmap(FILE *out, char *args)
{
	char *tok_seg, *remainder;

	int seg = ui_cmd_gettok_int(args, &tok_seg, &remainder);
	if (!tok_seg) {
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "Missing argument (memory segment)");
		return;
	}
	if ((seg < 0) || (seg > 15)) {
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "Wrong segment number");
		return;
	}

	int map = ectl_mem_map(seg);
	ui_cmd_resp(out, RESP_OK, UI_EOL, "0x%04x", map);
}

// -----------------------------------------------------------------------
void ui_cmd_int(FILE *out, char *args)
{
	char *tok_int, *remainder;

	int interrupt = ui_cmd_gettok_int(args, &tok_int, &remainder);

	// show interrupts
	if (!tok_int) {
		ui_cmd_resp(out, RESP_OK, UI_EOL, "0x%08x", ectl_int_get());
		return;
	}

	// set interrupt
	int res = ectl_int_set(interrupt);
	if (res) {
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "Wrong interrupt number: %s", tok_int);
	} else {
		ui_cmd_resp(out, RESP_OK, UI_EOL, "0x%08x", ectl_int_get());
	}
}

// -----------------------------------------------------------------------
void ui_cmd_oprq(FILE *out, char *args)
{
	ectl_oprq();
	ui_cmd_resp(out, RESP_OK, UI_EOL, "OPRQ");
}

// -----------------------------------------------------------------------
void ui_cmd_state(FILE *out, char *args)
{
	ui_cmd_resp(out, RESP_OK, UI_EOL, "0x%04x", ectl_cpu_state_get());
}

// -----------------------------------------------------------------------
void ui_cmd_quit(FILE *out, char *args)
{
	ectl_cpu_quit();
	ui_cmd_resp(out, RESP_OK, UI_EOL, "QUIT");
}

// -----------------------------------------------------------------------
void ui_cmd_log(FILE *out, char *args)
{
	char *tok_state, *remainder;

	int state = ui_cmd_gettok_bool(args, &tok_state, &remainder);

	// show logging state
	if (!tok_state) {
		ui_cmd_resp(out, RESP_OK, UI_EOL, "LOG %s", ectl_log_state_get() ? "ON" : "OFF");
		return;
	}

	// check state for correctness
	if (state < 0) {
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "Wrong state: %s", tok_state);
		return;
	}

	// set logging state
	if (!ectl_log_state_set(state)) {
		ui_cmd_resp(out, RESP_OK, UI_EOL, "LOG %s", ectl_log_state_get() ? "ON" : "OFF");
	} else {
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "Could not set logging state. LOG is now %s", ectl_log_state_get() ? "ON" : "OFF");
	}
}

// -----------------------------------------------------------------------
void ui_cmd_loglvl(FILE *out, char *args)
{
	char *tok_comp, *tok_lvl, *remainder;

	ui_cmd_gettok_str(args, &tok_comp, &remainder);

	// print all levels
	if (!tok_comp) {
		ui_cmd_resp(out, RESP_OK, UI_NOEOL, "");
		for (int i=0 ; i<ECTL_LOG_COUNT ; i++) {
			fprintf(out, " %s=%i", ectl_log_component_name(i), ectl_log_level_get(i));
		}
		fprintf(out, "\n");
		return;
	}

	// find component
	int component = ectl_log_component_id(tok_comp);
	if ((component < 0) || (component >= ECTL_LOG_COUNT)) {
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "Unknown component name: %s", tok_comp);
		return;
	}

	int level = ui_cmd_gettok_int(remainder, &tok_lvl, &remainder);

	// print level for component
	if (!tok_lvl) {
		ui_cmd_resp(out, RESP_OK, UI_EOL, "%s=%i", ectl_log_component_name(component), ectl_log_level_get(component));
		return;
	}

	// check level
	if ((level < ECTL_LOG_LEVEL_MIN) || (level > ECTL_LOG_LEVEL_MAX)) {
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "Wrong level: %s", tok_lvl);
		return;
	}

	// set level for component
	if (!ectl_log_level_set(component, level)) {
		ui_cmd_resp(out, RESP_OK, UI_EOL, "%s=%i", ectl_log_component_name(component), ectl_log_level_get(component));
	} else {
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "Cannot set level %i for component %s", ectl_log_level_get(component), ectl_log_component_name(component));
	}
}

// -----------------------------------------------------------------------
void ui_cmd_ips(FILE *out, char *args)
{
	ui_cmd_resp(out, RESP_OK, UI_EOL, "%li", ectl_ips_get());
}

// -----------------------------------------------------------------------
void ui_cmd_na(FILE *out, char *args)
{
	ui_cmd_resp(out, RESP_ERR, UI_EOL, "Command not implemented");
}

// -----------------------------------------------------------------------
struct ui_cmd_command commands[] = {
	{ "bin",	"<val>",					"Initiate binary load",				ui_cmd_na },
	{ "brk",	"<expr>",					"Set breakpoint",					ui_cmd_na },
	{ "brkdel",	"<num>",					"Delete breakpoint",				ui_cmd_na },
	{ "brkls",	"[num]",					"Show breakpoints",					ui_cmd_na },
	{ "clear",	"",							"CPU clear (reset)",				ui_cmd_clear },
	{ "clock",	"[on|off]",					"Show, enable, disable clock",		ui_cmd_clock },
	{ "cycle",	"",							"Execute one CPU cycle",			ui_cmd_cycle },
	{ "dasm",	"<seg> <addr>",				"Disassemble instruction",			ui_cmd_na },
	{ "eval",	"<expr>",					"Evaluate expression",				ui_cmd_na },
	{ "help",	"",							"Show help",						ui_cmd_help },
	{ "info",	"",							"Show system info",					ui_cmd_info },
	{ "int",	"[interrupt]",				"Show interrupts, send irq",		ui_cmd_int },
	{ "ips",	"",							"Show IPS",							ui_cmd_ips },
	{ "load",	"<seg> <addr> <file>",		"Load file to memory",				ui_cmd_load },
	{ "log",	"[on|off]",					"Show, enable, disable logging",	ui_cmd_log },
	{ "loglvl",	"[component [level]]",		"Manipulate logging levels",		ui_cmd_loglvl },
	{ "mem",	"<seg> <addr> [count]",		"Get memory contents",				ui_cmd_mem },
	{ "memfind","<seg> <val>",				"Search memory contents",			ui_cmd_na },
	{ "memmap",	"<seg>",					"Show memory allocation",			ui_cmd_memmap },
	{ "memw",	"<seg> <addr> <val> ...",	"Set memory contents",				ui_cmd_memw },
	{ "oprq",	"",							"Send operator request",			ui_cmd_oprq },
	{ "quit",	"",							"Quit emulation",					ui_cmd_quit },
	{ "reg",	"[name [value]]",			"Manipulate registers",				ui_cmd_reg },
	{ "run",	"",							"Start CPU",						ui_cmd_start },
	{ "start",	"",							"Start CPU",						ui_cmd_start },
	{ "state",	"",							"Show CPU state",					ui_cmd_state },
	{ "stop",	"",							"Stop CPU",							ui_cmd_stop },
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
