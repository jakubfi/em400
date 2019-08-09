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

#ifndef CFG_H
#define CFG_H

extern int cfg_error;

// -----------------------------------------------------------------------
struct cfg_arg {
	char *text;
	struct cfg_arg *next;
};

// -----------------------------------------------------------------------
struct cfg_unit {
	char *name;
	int num;
	struct cfg_arg *args;
	struct cfg_unit *next;
};

// -----------------------------------------------------------------------
struct cfg_chan {
	char *name;
	int num;
	struct cfg_unit *units;
	struct cfg_chan *next;
};

// -----------------------------------------------------------------------
struct cfg_em400 {

	char *program_name;
	char *cfg_filename;
	int print_help;
	char *ui_name;

	int speed_real;
	int clock_period;
	int clock_start;
	int cpu_mod;
	int cpu_user_io_illegal;
	int cpu_awp;
	int keys;
	int mem_elwro;
	int mem_mega;
	char *mem_mega_prom;
	int mem_mega_boot;
	int cpu_stop_on_nomem;
	int mem_os;
	int log_enabled;
	char *log_file;
	char *log_components;
	int line_buffered;

	int fpga;
	char *fpga_dev;
	int fpga_speed;

	struct cfg_chan *chans;
};

struct cfg_em400 * cfg_create_default();
void cfg_destroy(struct cfg_em400 *cfg);
int cfg_from_args(struct cfg_em400 *cfg, int argc, char **argv);
int cfg_from_file(struct cfg_em400 *cfg);

struct cfg_arg * cfg_make_arg(char *arg);
int cfg_args_decode(struct cfg_arg *arg, const char *format, ...);
void cfg_make_unit(struct cfg_em400 *cfg, int u_num, char *name, struct cfg_arg *args);
void cfg_make_chan(struct cfg_em400 *cfg, int c_num, char *name);
void cfg_drop_chans(struct cfg_chan *c);

void cfg_log(struct cfg_em400 *cfg);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
