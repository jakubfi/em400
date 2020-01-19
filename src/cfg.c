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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include "mem/mem.h"
#include "io/defs.h"
#include "cfg.h"
#include "cfg_parser.h"
#include "log.h"

extern FILE *cyyin;
void cyyerror(const char *s, ...);
int cyyparse();
int cyylex_destroy();
int cfg_error = 0;

// -----------------------------------------------------------------------
struct cfg_em400 * cfg_create_default()
{
	struct cfg_em400 *cfg = (struct cfg_em400 *) malloc(sizeof(struct cfg_em400));
	if (!cfg) return NULL;

	const char *cfile = "/.em400/em400.cfg";
	char *home = getenv("HOME");
	char *home_cfg_fname = (char *) malloc(strlen(home) + strlen(cfile) + 1);
	if (!home_cfg_fname) {
		cfg_destroy(cfg);
		return NULL;
	}
	sprintf(home_cfg_fname, "%s%s", home, cfile);

	// emulator
	cfg->program_name = NULL;
	cfg->cfg_filename = home_cfg_fname;
	cfg->print_help = 0;
	cfg->ui_name = strdup("curses");

	// emulation
	cfg->speed_real = 0;
	cfg->cpu_speed_factor = 1;
	cfg->throttle_granularity = 4;
	cfg->buzzer = 0;

	// cpu
	cfg->clock_period= 10;
	cfg->clock_start = 1;
	cfg->cpu_mod = 0;
	cfg->cpu_user_io_illegal = 1;
	cfg->cpu_awp = 1;
	cfg->cpu_stop_on_nomem = 1;

	// control panel
	cfg->keys = 0;

	// mem
	cfg->mem_elwro = 1;
	cfg->mem_mega = 0;
	cfg->mem_os = 2;
	cfg->mem_mega_prom = NULL;
	cfg->mem_mega_boot = 0;

	// log
	cfg->log_enabled = 0;
	cfg->log_file = strdup("em400.log");
	cfg->log_components = strdup("");
	cfg->line_buffered = 1;

	// FPGA
	cfg->fpga = 0;
	cfg->fpga_dev = strdup("/dev/ttyUSB0");
	cfg->fpga_speed = 1000000;

	// I/O
	cfg->chans = NULL;

	return cfg;
}

// -----------------------------------------------------------------------
void cfg_destroy(struct cfg_em400 *cfg)
{
	if (!cfg) return;

	free(cfg->ui_name);
	free(cfg->program_name);
	free(cfg->cfg_filename);
	free(cfg->mem_mega_prom);
	free(cfg->log_file);
	free(cfg->log_components);
	free(cfg->fpga_dev);
	cfg_drop_chans(cfg->chans);
	free(cfg);
}

// -----------------------------------------------------------------------
int cfg_from_args(struct cfg_em400 *cfg, int argc, char **argv)
{
	int option;
	optind = 1; // reset to 1 so consecutive calls work

	while ((option = getopt(argc, argv, "hc:p:k:l:Lu:F")) != -1) {
		switch (option) {
			case 'L':
				cfg->log_enabled = 0;
				break;
			case 'l':
				cfg->log_enabled = 1;
				free(cfg->log_components);
				cfg->log_components = strdup(optarg);
				break;
			case 'h':
				cfg->print_help = 1;
				break;
			case 'c':
				free(cfg->cfg_filename);
				cfg->cfg_filename = strdup(optarg);
				break;
			case 'p':
				free(cfg->program_name);
				cfg->program_name = strdup(optarg);
				break;
			case 'k':
				cfg->keys = atoi(optarg);
				break;
			case 'u':
				free(cfg->ui_name);
				cfg->ui_name = strdup(optarg);
				break;
			case 'F':
				cfg->fpga = 1;
				break;
			default:
				return E_ERR;
		}
	}

	return E_OK;
}

// -----------------------------------------------------------------------
int cfg_from_file(struct cfg_em400 *cfg)
{
	FILE *cfgf = fopen(cfg->cfg_filename, "r");
	if (!cfgf) {
		return E_ERR;
	}

	cyyin = cfgf;
	do {
		cyyparse(cfg);
	} while (!feof(cyyin));

	fclose(cfgf);
	cyylex_destroy();

	if (cfg_error) {
		return E_ERR;
	}

	return E_OK;
}

// -----------------------------------------------------------------------
struct cfg_arg * cfg_make_arg(char *s)
{
	struct cfg_arg *a = (struct cfg_arg *) malloc(sizeof(struct cfg_arg));
	a->text = s;
	a->next = NULL;
	return a;
}

// -----------------------------------------------------------------------
int cfg_args_decode(struct cfg_arg *arg, const char *format, ...)
{
	int *i;
	char *c;
	char **s;
	char *eptr = NULL;

	assert(format);

	va_list ap;
	va_start(ap, format);
	while (*format) {
		assert(arg);
		assert(arg->text);
		switch (*format) {
			case 'i':
				i = va_arg(ap, int*);
				*i = strtol(arg->text, &eptr, 10);
				if (*eptr != '\0') {
					return E_ERR;
				}
				break;
			case 'c':
				c = va_arg(ap, char*);
				*c = *(arg->text);
				break;
			case 's':
				s = va_arg(ap, char**);
				*s = strdup(arg->text);
				if (!*s) {
					return LOGERR("Memory allocation error while parsing configuration arguments.");
				}
				break;
			default:
				assert(!"Format unknown to cfg_args_decode()");
				return E_ERR;
		}
		format++;
		arg = arg->next;
	}

	va_end(ap);

	return E_OK;
}

// -----------------------------------------------------------------------
void cfg_make_unit(struct cfg_em400 *cfg, int u_num, char *name, struct cfg_arg *arglist)
{
	if (u_num < 0) {
		cyyerror("Incorrect unit number: %i", u_num);
	}

	struct cfg_unit *u = (struct cfg_unit *) malloc(sizeof(struct cfg_unit));
	u->name = name;
	u->num = u_num;
	u->args = arglist;
	u->next = cfg->chans->units;
	cfg->chans->units = u;
}

// -----------------------------------------------------------------------
void cfg_make_chan(struct cfg_em400 *cfg, int c_num, char *name)
{
	if ((c_num < 0) || (c_num > IO_MAX_CHAN)) {
		cyyerror("Incorrect channel number: %i", c_num);
	}

	struct cfg_chan *c = (struct cfg_chan *) malloc(sizeof(struct cfg_chan));
	c->num = c_num;
	c->name = name;
	c->next = cfg->chans;
	c->units = NULL;
	cfg->chans = c;
}

// -----------------------------------------------------------------------
void cfg_drop_args(struct cfg_arg *a)
{
	if (!a) {
		return;
	}
	cfg_drop_args(a->next);
	free(a->text);
	free(a);
}

// -----------------------------------------------------------------------
void cfg_drop_units(struct cfg_unit *u)
{
	if (!u) {
		return;
	}
	cfg_drop_units(u->next);
	cfg_drop_args(u->args);
	free(u->name);
	free(u);
}

// -----------------------------------------------------------------------
void cfg_drop_chans(struct cfg_chan *c)
{
	if (!c) {
		return;
	}
	cfg_drop_chans(c->next);
	cfg_drop_units(c->units);
	free(c->name);
	free(c);
}

// -----------------------------------------------------------------------
void cfg_log(struct cfg_em400 *cfg)
{
	LOG(L_EM4H, "---- Effective configuration: ----------");
	LOG(L_EM4H, "Program to load: %s", cfg->program_name);
	LOG(L_EM4H, "Loaded config: %s", cfg->cfg_filename);
	LOG(L_EM4H, "Print help: %s", cfg->print_help ? "yes" : "no");
	LOG(L_EM4H, "Use FPGA backend: %s", cfg->fpga ? "yes" : "no");
	if (!cfg->fpga) {
		LOG(L_EM4H, "CPU emulation:");
		LOG(L_EM4H, "   Emulation speed: %s", cfg->speed_real ? "real" : "maximum");
		LOG(L_EM4H, "   CPU speed factor: %f", cfg->cpu_speed_factor);
		LOG(L_EM4H, "   Throttle granularity: %i CPU cycles", cfg->throttle_granularity);
		LOG(L_EM4H, "   Buzzer emulation: %s", cfg->buzzer ? "yes" : "no");
		LOG(L_EM4H, "   Clock period: %i (%s at power-on)", cfg->clock_period, cfg->clock_start ? "enabled" : "disabled");
		LOG(L_EM4H, "   CPU modifications: %s", cfg->cpu_mod ? "present" : "absent");
		LOG(L_EM4H, "   IN/OU instructions: %s for user programs", cfg->cpu_user_io_illegal ? "illegal" : "legal");
		LOG(L_EM4H, "   Hardware AWP: %s", cfg->cpu_awp ? "present" : "absent");
		LOG(L_EM4H, "   CPU stop on nomem in OS block: %s", cfg->cpu_stop_on_nomem ? "yes" : "no");
		LOG(L_EM4H, "Memory emulation:");
		LOG(L_EM4H, "   Elwro modules: %i", cfg->mem_elwro);
		LOG(L_EM4H, "   MEGA modules: %i", cfg->mem_mega);
		LOG(L_EM4H, "   MEGA PROM image: %s", cfg->mem_mega_prom);
		LOG(L_EM4H, "   MEGA boot: %s", cfg->mem_mega_boot ? "true" : "false");
		LOG(L_EM4H, "   Hardwired segments for OS: %i", cfg->mem_os);
	} else {
		LOG(L_EM4H, "FPGA backend:");
		LOG(L_EM4H, "   Device: %s", cfg->fpga_dev);
		LOG(L_EM4H, "   Link speed: %i", cfg->fpga_speed);
	}
	LOG(L_EM4H, "KB: 0x%04x", cfg->keys);
	LOG(L_EM4H, "Logging (%s):", cfg->log_enabled ? "enabled" : "disabled");
	LOG(L_EM4H, "   File: %s", cfg->log_file);
	LOG(L_EM4H, "   Components: %s", cfg->log_components);
	LOG(L_EM4H, "   Buffering: %s", cfg->line_buffered ? "line" : "full");
	LOG(L_EM4H, "I/O:");

	char buf[4096];
	int bpos;

	struct cfg_chan *chanc = cfg->chans;
	while (chanc) {
		LOG(L_EM4H, "   Channel %2i: %s", chanc->num, chanc->name);

		struct cfg_unit *unitc = chanc->units;
		while (unitc) {
			bpos = 0;
			struct cfg_arg *args = unitc->args;
			while (args) {
				bpos += sprintf(buf+bpos, "%s", args->text);
				args = args->next;
				if (args) {
					bpos += sprintf(buf+bpos, ", ");
				}
			}
			LOG(L_EM4H, "      Unit %2i: %s (%s)", unitc->num, unitc->name, buf);
			unitc = unitc->next;
		}

		chanc = chanc->next;
	}
	LOG(L_EM4H, "----------------------------------------");
}

// vim: tabstop=4 shiftwidth=4 autoindent
