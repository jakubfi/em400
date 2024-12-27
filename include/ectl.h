//  Copyright (c) 2016-2024 Jakub Filipowicz <jakubf@gmail.com>
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

#pragma once

#ifndef ECTL_H
#define ECTL_H

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

enum ectl_cpu_states {
	ECTL_STATE_RUN = 0,
	ECTL_STATE_STOP,
	ECTL_STATE_WAIT,
	ECTL_STATE_CLO,
	ECTL_STATE_OFF,
	ECTL_STATE_CYCLE,
	ECTL_STATE_BIN,
	ECTL_STATE_LOAD,
	ECTL_STATE_STORE,
	ECTL_STATE_FETCH,
// NOTE: update state names in ectl_cpu_state_get() when adding new states
	ECTL_STATE_ANY,
	ECTL_STATE_UNKNOWN,
};

extern const char *state_names[];

int ectl_init();
void ectl_shutdown();

// Standard control panel interface
void ectl_cpu_off(); // TODO: on + led?

// memory
bool ectl_mem_read_n(int seg, uint16_t addr, uint16_t *dest, unsigned count);
bool ectl_mem_write_n(int seg, uint16_t addr, uint16_t *src, unsigned count);
int ectl_mem_map(int seg);
bool ectl_load_os_image(FILE *f, const char *name, int seg, uint16_t saddr);

// CPU state
const char * ectl_cpu_state_name(unsigned state);
unsigned ectl_cpu_state_get();

// informational, other
int ectl_eval(char *expression, char **err_msg, int *err_beg, int *err_end);

// breakpoints
int ectl_brk_add(char *expression, char **err_msg, int *err_beg, int *err_end);
int ectl_brk_del(unsigned id);

#ifdef __cplusplus
}
#endif

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
