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
#include "libem400.h"

#include "ui/cmd/commands.h"
#include "ui/cmd/utils.h"

void ui_cmd_state(FILE *out, char *args);
void ui_cmd_reg(FILE *out, char *args);
void ui_cmd_int(FILE *out, char *args);
void ui_cmd_mem(FILE *out, char *args);
void ui_cmd_bin(FILE *out, char *args);
void ui_cmd_eval(FILE *out, char *args);
void ui_cmd_ips(FILE *out, char *args);
void ui_cmd_load(FILE *out, char *args);
void ui_cmd_clock(FILE *out, char *args);
void ui_cmd_oprq(FILE *out, char *args);
void ui_cmd_memw(FILE *out, char *args);
void ui_cmd_cycle(FILE *out, char *args);
void ui_cmd_start(FILE *out, char *args);
void ui_cmd_stop(FILE *out, char *args);
void ui_cmd_clear(FILE *out, char *args);
void ui_cmd_memmap(FILE *out, char *args);
void ui_cmd_log(FILE *out, char *args);
void ui_cmd_logc(FILE *out, char *args);
void ui_cmd_info(FILE *out, char *args);
void ui_cmd_quit(FILE *out, char *args);
void ui_cmd_help(FILE *out, char *args);
void ui_cmd_brk(FILE *out, char *args);
void ui_cmd_brkdel(FILE *out, char *args);
void ui_cmd_stopn(FILE *out, char *args);

struct ui_cmd_command commands[] = {
	{ UI_CMD_FLAG_NONE, "state",	"",							"Get CPU state",					ui_cmd_state },
	{ UI_CMD_FLAG_NONE, "reg",		"[name [value]]",			"Manipulate registers",				ui_cmd_reg },
	{ UI_CMD_FLAG_NONE, "int",		"[interrupt]",				"Get interrupts, send IRQ",			ui_cmd_int },
	{ UI_CMD_FLAG_NONE, "mem",		"<seg> <addr> [count]",		"Get memory contents",				ui_cmd_mem },
	{ UI_CMD_FLAG_NONE, "eval",		"<expr>",					"Evaluate expression",				ui_cmd_eval },
	{ UI_CMD_FLAG_NONE, "ips",		"",							"Get average IPS",					ui_cmd_ips },
	{ UI_CMD_FLAG_NONE, "load",		"<seg> <addr> <file>",		"Load file into memory",			ui_cmd_load },
	{ UI_CMD_FLAG_NONE, "bin",		"<cmd> <addr>",				"Initiate binary load",				ui_cmd_bin },
	{ UI_CMD_FLAG_NONE, "brk",		"<expr>",					"Add breakpoint",					ui_cmd_brk },
	{ UI_CMD_FLAG_NONE, "brkdel",	"<id>",						"Delete breakpoint",				ui_cmd_brkdel },
	{ UI_CMD_FLAG_NONE, "stopn",	"<addr>|off",				"Stop CPU on address",				ui_cmd_stopn },
	{ UI_CMD_FLAG_NONE, "clock",	"[on|off]",					"Manipulate clock state",			ui_cmd_clock },
	{ UI_CMD_FLAG_NONE, "oprq",		"",							"Send operator request",			ui_cmd_oprq },
	{ UI_CMD_FLAG_NONE, "memw",		"<seg> <addr> <val> ...",	"Set memory contents",				ui_cmd_memw },
	{ UI_CMD_FLAG_NONE, "cycle",	"",							"Execute one CPU cycle",			ui_cmd_cycle },
	{ UI_CMD_FLAG_NONE, "start",	"",							"Start CPU",						ui_cmd_start },
	{ UI_CMD_FLAG_NONE, "stop",		"",							"Stop CPU",							ui_cmd_stop },
	{ UI_CMD_FLAG_NONE, "clear",	"",							"Clear CPU (reset)",				ui_cmd_clear },
	{ UI_CMD_FLAG_NONE, "memmap",	"<seg>",					"Get memory allocation map",		ui_cmd_memmap },
	{ UI_CMD_FLAG_NONE, "log",		"[on|off]",					"Manipulate logging state",			ui_cmd_log },
	{ UI_CMD_FLAG_NONE, "logc",		"[component [state]]",		"Manipulate log compoment state",	ui_cmd_logc },
	{ UI_CMD_FLAG_NONE, "info",		"",							"Get emulator info",				ui_cmd_info },
	{ UI_CMD_FLAG_QUIT, "quit",		"",							"Quit emulation",					ui_cmd_quit },
	{ UI_CMD_FLAG_NONE, "help",		"",							"Get help",							ui_cmd_help },
	{ UI_CMD_FLAG_NONE, NULL, NULL, NULL, NULL },
};

// -----------------------------------------------------------------------
void ui_cmd_resp(FILE *out, int status, int eol, const char *fmt, ...)
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
		uint16_t regs[EM400_REG_COUNT];
		em400_regs(regs);

		ui_cmd_resp(out, RESP_OK, UI_NOEOL, "");
		for (int i=0 ; i<EM400_REG_COUNT ; i++) {
			fprintf(out, " %s=0x%04x", em400_reg_name(i), regs[i]);
		}
		fprintf(out, "\n");
		return;
	}

	// find register
	int id = em400_reg_id(tok_reg);
	if (id < 0) {
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "No such register: %s", tok_reg);
		return;
	}

	int value = ui_cmd_gettok_int(remainder, &tok_val, &remainder);

	// show specific register
	if (!tok_val) {
		ui_cmd_resp(out, RESP_OK, UI_EOL, "0x%04x", em400_reg(id));
		return;
	}

	// check value
	if (value < 0) {
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "Value is not a valid 16-bit integer: %s", tok_val);
		return;
	}

	if (id == EM400_REG_KB) {
		em400_cp_kb(value);
	} else {
		em400_reg_set(id, value);
	}

	ui_cmd_resp(out, RESP_OK, UI_EOL, "0x%04x", value);
}

// -----------------------------------------------------------------------
static int cmpstringp(const void *p1, const void *p2)
{
	return strcmp(* (char * const *) p1, * (char * const *) p2);
}

// -----------------------------------------------------------------------
void ui_cmd_help(FILE *out, char *args)
{
	char *tok_cmd, *remainder;

	ui_cmd_gettok_str(args, &tok_cmd, &remainder);

	// show all commands
	if (!tok_cmd) {
		int nelem = sizeof(commands) / sizeof(struct ui_cmd_command);
		const char **cmdnames = (const char **) malloc(nelem * sizeof(char*));

		int i=0;
		struct ui_cmd_command *c = commands;
		while (c && c->name && (i < nelem)) {
			cmdnames[i] = c->name;
			c++;
			i++;
		}
		qsort(cmdnames, i, sizeof(char*), cmpstringp);

		ui_cmd_resp(out, RESP_OK, UI_NOEOL, " Available commands:");
		for (int j=0 ; j<i ; j++) {
			fprintf(out, " %s", (char *)cmdnames[j]);
		}
		fprintf(out, "\n");

		free(cmdnames);
		return;
	}

	struct ui_cmd_command *cmd = ui_cmd_find_command(tok_cmd);

	// no such command
	if (!cmd) {
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "No help for command: %s", tok_cmd);
		return;
	}

	// show specific command help
	if (*cmd->args) {
		ui_cmd_resp(out, RESP_OK, UI_EOL, "%s %s : %s", cmd->name, cmd->args, cmd->desc);
	} else {
		ui_cmd_resp(out, RESP_OK, UI_EOL, "%s : %s", cmd->name, cmd->desc);
	}
}
// -----------------------------------------------------------------------
void ui_cmd_info(FILE *out, char *args)
{
	ui_cmd_resp(out, RESP_OK, UI_EOL, " EM400 %s", em400_version());
}

// -----------------------------------------------------------------------
void ui_cmd_clock(FILE *out, char *args)
{
	char *tok_state, *remainder;

	int state = ui_cmd_gettok_bool(args, &tok_state, &remainder);

	// show clock state
	if (!tok_state) {
		ui_cmd_resp(out, RESP_OK, UI_EOL, "%i", em400_cp_clock_led());
		return;
	}

	// is state correct?
	if (state < 0) {
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "Wrong state: %s", tok_state);
		return;
	}

	// set clock state
	em400_cp_clock(state);
	ui_cmd_resp(out, RESP_OK, UI_EOL, "%i", state);
}

// -----------------------------------------------------------------------
void ui_cmd_stop(FILE *out, char *args)
{
	em400_cp_start(false);
	ui_cmd_resp(out, RESP_OK, UI_EOL, "STOP");
}

// -----------------------------------------------------------------------
void ui_cmd_start(FILE *out, char *args)
{
	em400_cp_start(true);
	ui_cmd_resp(out, RESP_OK, UI_EOL, "START");
}

// -----------------------------------------------------------------------
void ui_cmd_cycle(FILE *out, char *args)
{
	em400_cp_cycle();
	ui_cmd_resp(out, RESP_OK, UI_EOL, "CYCLE");
}

// -----------------------------------------------------------------------
void ui_cmd_clear(FILE *out, char *args)
{
	em400_cp_clear();
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
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "Wrong segment number: %i", seg);
		return;
	}

	int addr = ui_cmd_gettok_int(remainder, &tok_addr, &remainder);
	if (!tok_addr) {
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "Missing argument (start address)");
		return;
	}
	if (addr < 0) {
		ui_cmd_resp(out, RESP_ERR, UI_EOL,  "Invalid address: %i", addr);
		return;
	}

	int count = ui_cmd_gettok_int(remainder, &tok_count, &remainder);
	if (!tok_count) {
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "Missing argument (word count)");
		return;
	}
	if (count < 1) {
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "Invalid word count: %i", count);
		return;
	}

	uint16_t *mbuf = (uint16_t *) malloc(count * sizeof(uint16_t));
	int processed = 0;

	ui_cmd_resp(out, RESP_OK, UI_NOEOL, "");

	while (processed < count) {
		int words = count - processed;
		if (!em400_mem_read(seg, addr+processed, mbuf, words)) {
			ui_cmd_resp(out, RESP_ERR, UI_EOL, "Memory read failed");
			return;
		}
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
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "Wrong segment number: %i", seg);
		return;
	}

	int addr = ui_cmd_gettok_int(remainder, &tok_addr, &remainder);
	if (!tok_addr) {
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "Missing argument (start address)");
		return;
	}
	if (addr < 0) {
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "Invalid address: %i", addr);
		return;
	}

	uint16_t mbuf[0x10000];
	int processed = 0;

	do {
		int val = ui_cmd_gettok_int(remainder, &tok_val, &remainder);
		if (tok_val) {
			if (val < 0) {
				ui_cmd_resp(out, RESP_ERR, UI_EOL, "Value on position %i is not a valid 16-bit integer: %i", processed, val);
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

	if (!em400_mem_write(seg, addr, mbuf, processed)) {
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "Memory write failed when writing %i words at address 0x%04x", processed, addr);
		return;
	}

	ui_cmd_resp(out, RESP_OK, UI_EOL, "%i words written", processed);
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
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "Wrong segment number: %i", seg);
		return;
	}

	int addr = ui_cmd_gettok_int(remainder, &tok_addr, &remainder);
	if (!tok_addr) {
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "Missing argument (start address)");
		return;
	}
	if (addr < 0) {
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "Invalid address: %i", addr);
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
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "Error opening file: %s", tok_file);
		return;
	}

	bool res = em400_load_os_image(f);
	if (!res) {
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "File upload failed: %s", tok_file);
	} else {
		ui_cmd_resp(out, RESP_OK, UI_EOL, "%i words loaded from file %s", res, tok_file);
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
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "Wrong segment number: %i", seg);
		return;
	}

	int map = em400_mem_map(seg);
	ui_cmd_resp(out, RESP_OK, UI_EOL, "0x%04x", map);
}

// -----------------------------------------------------------------------
void ui_cmd_int(FILE *out, char *args)
{
	char *tok_int, *remainder;

	int interrupt = ui_cmd_gettok_int(args, &tok_int, &remainder);

	// show interrupts
	if (!tok_int) {
		ui_cmd_resp(out, RESP_OK, UI_EOL, "0x%08x", em400_rz32());
		return;
	}

	// set interrupt
	int res = em400_int_set(interrupt);
	if (res) {
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "Wrong interrupt number: %s", tok_int);
	} else {
		ui_cmd_resp(out, RESP_OK, UI_EOL, "0x%08x", em400_rz32());
	}
}

// -----------------------------------------------------------------------
void ui_cmd_oprq(FILE *out, char *args)
{
	em400_cp_oprq();
	ui_cmd_resp(out, RESP_OK, UI_EOL, "OPRQ");
}

// -----------------------------------------------------------------------
void ui_cmd_state(FILE *out, char *args)
{
	const char *state = em400_cpu_state_name(em400_cpu_state());
	ui_cmd_resp(out, RESP_OK, UI_EOL, "%s", state);
}

// -----------------------------------------------------------------------
void ui_cmd_quit(FILE *out, char *args)
{
	ectl_cpu_off();
	ui_cmd_resp(out, RESP_OK, UI_EOL, "QUIT");
}

// -----------------------------------------------------------------------
void ui_cmd_log(FILE *out, char *args)
{
	char *tok_state, *remainder;

	int state = ui_cmd_gettok_bool(args, &tok_state, &remainder);

	// show logging state
	if (!tok_state) {
		ui_cmd_resp(out, RESP_OK, UI_EOL, "%i", em400_log_state());
		return;
	}

	// check state for correctness
	if (state < 0) {
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "Wrong state: %s", tok_state);
		return;
	}

	// set logging state
	if (!em400_log_set(state)) {
		ui_cmd_resp(out, RESP_OK, UI_EOL, "%i", em400_log_state());
	} else {
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "Could not set logging state. LOG state is now %i", em400_log_state());
	}
}

// -----------------------------------------------------------------------
void ui_cmd_logc(FILE *out, char *args)
{
	char *tok_comp, *tok_lvl, *remainder;

	ui_cmd_gettok_str(args, &tok_comp, &remainder);

	// print all components
	if (!tok_comp) {
		ui_cmd_resp(out, RESP_OK, UI_NOEOL, "");
		for (int i=0 ; i<L_COUNT ; i++) {
			if (em400_log_component_state(i)) {
				fprintf(out, " %s", em400_log_component_name(i));
			}
		}
		fprintf(out, "\n");
		return;
	}

	// find component
	int component = em400_log_component_id(tok_comp);
	if ((component < 0) || (component >= L_COUNT)) {
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "Unknown component name: %s", tok_comp);
		return;
	}

	int state = ui_cmd_gettok_int(remainder, &tok_lvl, &remainder);

	// print component state
	if (!tok_lvl) {
		ui_cmd_resp(out, RESP_OK, UI_EOL, "%i", em400_log_component_state(component));
		return;
	}

	// set component state
	em400_log_component_set(component, state);
	ui_cmd_resp(out, RESP_OK, UI_EOL, "%i", em400_log_component_state(component));
}

// -----------------------------------------------------------------------
void ui_cmd_ips(FILE *out, char *args)
{
	ui_cmd_resp(out, RESP_OK, UI_EOL, "%li", em400_ips_get());
}

// -----------------------------------------------------------------------
void ui_cmd_eval(FILE *out, char *args)
{
	char *tok_expr = ui_cmd_skip_ws(args);
	if (!tok_expr || !*tok_expr) {
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "Missing argument (expression)");
		return;
	}

	char *error_msg = NULL;
	int err_beg, err_end;
	int res = ectl_eval(tok_expr, &error_msg, &err_beg, &err_end);
	if (error_msg) {
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "%s (at %i-%i)", error_msg, err_beg, err_end);
		free(error_msg);
	} else {
		ui_cmd_resp(out, RESP_OK, UI_EOL, "%i", res);
	}
}

// -----------------------------------------------------------------------
void ui_cmd_brk(FILE *out, char *args)
{
	char *tok_expr = ui_cmd_skip_ws(args);
	if (!tok_expr || !*tok_expr) {
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "Missing argument (expression)");
		return;
	}

	char *error_msg = NULL;
	int err_beg, err_end;
	int id = ectl_brk_add(tok_expr, &error_msg, &err_beg, &err_end);
	if (error_msg) {
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "%s (at %i-%i)", error_msg, err_beg, err_end);
		free(error_msg);
		return;
	}

	ui_cmd_resp(out, RESP_OK, UI_EOL, "Added breakpoint: %i", id);
}

// -----------------------------------------------------------------------
void ui_cmd_brkdel(FILE *out, char *args)
{
	char *tok_brk, *remainder;

	int brk_num = ui_cmd_gettok_int(args, &tok_brk, &remainder);
	if (!tok_brk) {
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "Missing argument (breakpoint number)");
		return;
	}

	int res = ectl_brk_del(brk_num);
	if (res) {
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "No such breakpoint");
		return;
	}
	ui_cmd_resp(out, RESP_OK, UI_EOL, "Removed breakpoint: %i", brk_num);
}

// -----------------------------------------------------------------------
void ui_cmd_bin(FILE *out, char *args)
{
	em400_cp_bin();
	ui_cmd_resp(out, RESP_OK, UI_EOL, "Binary load");
}

// -----------------------------------------------------------------------
void ui_cmd_stopn(FILE *out, char *args)
{
	char *tok_addr, *remainder;

	int addr = ui_cmd_gettok_int(args, &tok_addr, &remainder);
	if (!tok_addr) {
		ui_cmd_resp(out, RESP_ERR, UI_EOL, "Missing argument (address)");
		return;
	}

	em400_cp_stopn(addr);
	ui_cmd_resp(out, RESP_OK, UI_EOL, "Stop on address: 0x%04x", addr);

}

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
