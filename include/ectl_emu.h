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

#ifndef ECTL_H
#define ECTL_H

enum ectl_state {
	ECTL_OFF = 0,
	ECTL_ON = 1,
};

enum ectl_callbacks {
	ECTL_CB_STOP,
	ECTL_CB_HALT,
	ECTL_CB_MAX
};

enum ectl_log_components {
	ECTL_LOG_REG = 0,
	ECTL_LOG_MEM,
	ECTL_LOG_CPU,
	ECTL_LOG_OP,
	ECTL_LOG_INT,
	ECTL_LOG_IO,
	ECTL_LOG_MX,
	ECTL_LOG_PX,
	ECTL_LOG_CCHR,
	ECTL_LOG_CMEM,
	ECTL_LOG_TERM,
	ECTL_LOG_9425,
	ECTL_LOG_WNCH,
	ECTL_LOG_FLOP,
	ECTL_LOG_PNCH,
	ECTL_LOG_PNRD,
	ECTL_LOG_CRK5,
	ECTL_LOG_EM4H,
	ECTL_LOG_ALL
};

typedef int (*ectl_callback_f)(int v);

int ectl_callback_register(int callback, ectl_callback_f callback_f);
void ectl_log_state_set(int state);
int ectl_log_state_get();
int ectl_log_level_set(int component, int level);
int ectl_log_level_get(int component);
int ectl_cpu_count();
int ectl_mem_mega_prom_state_get();

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
