//  Copyright (c) 2024 Jakub Filipowicz <jakubf@gmail.com>
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

#ifndef __LIBEM400_H__
#define __LIBEM400_H__

#include <stdbool.h>

struct em400_cfg_cpu {
	bool awp;
	bool mod;
	bool user_io_illegal;
	bool nomem_stop;
	bool speed_real;
	int throttle_granularity;
	int clock_period;
};

struct em400_cfg_buzzer {
	bool enabled;
	const char *driver;
	const char *output;
	int volume;
	int sample_rate;
	int buffer_len;
	int latency;
};

struct em400_cfg_mem {
	int elwro_modules;
	int mega_modules;
	int os_segments;
	const char *mega_prom_image;
	const char *os_mem_preload; // ?
};

enum device_types {
	DEV_TERMINAL_TCP,
	DEV_TERMINAL_SERIAL,
	DEV_WINCHESTER,
	DEV_FLOP8,
};

struct em400_config_device {
	int chnum;
	int devnum;
	int type;
	const char *image;
	int port;
	int tansport_type;
};

int em400_mem_configure(struct em400_cfg_mem *c_mem);
int em400_cpu_configure(struct em400_cfg_cpu *c_cpu, struct em400_cfg_buzzer *c_buzzer);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
