//  Copyright (c) 2014 Jakub Filipowicz <jakubf@gmail.com>
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

#ifndef EMULOG_H
#define EMULOG_H

#include <inttypes.h>

#include "atomic.h"

enum emulog_components {
	L_REG, L_MEM, L_CPU, L_OP, L_INT,
	L_IO,
	L_MX, L_PX, L_CHAR, L_CMEM,
	L_TERM, L_WNCH, L_FLOP, L_PNCH, L_PNRD,
	L_CRK5,
	L_EM4H,
	L_MAX
};

extern int emulog_enabled;

int emulog_init(int paused, char *filename, int level, int pname_offset, int cpu_mod);
void emulog_shutdown();

void emulog_enable();
void emulog_disable();
int emulog_is_enabled();

int emulog_set_level(int component, unsigned level);
int emulog_get_level(unsigned component);
int emulog_wants(unsigned component, unsigned level);

char * emulog_get_component_name(unsigned component);
int emulog_get_component_id(char *name);

void emulog_log(unsigned component, unsigned level, char *format, ...);
void emulog_splitlog(unsigned component, unsigned level, char *text);

void emulog_store_cycle_state(uint16_t sr, uint16_t ic);
void emulog_intlevel_reset();
void emulog_intlevel_dec();
void emulog_intlevel_inc();

void emulog_log_dasm(unsigned component, unsigned level, int mod, int norm_arg, int short_arg, int16_t n);
void emulog_log_cpu(unsigned component, unsigned level, char *msgfmt, ...);

void emulog_update_pname();
void emulog_handle_sp(unsigned component, unsigned level, uint16_t n);
void emulog_handle_syscall(unsigned component, unsigned level, int number, int nb, int addr, int r4);
void emulog_handle_syscall_ret(unsigned component, unsigned level, uint16_t n);
void emulog_syscall_reset();

#define EMULOG_ENABLED (atom_load(&emulog_enabled))
#define EMULOG_WANTS(component, level) (EMULOG_ENABLED && emulog_wants(component, level))

#define EMULOG(component, level, format, ...) \
	if (EMULOG_WANTS(component, level)) \
		emulog_log(component, level, format, ##__VA_ARGS__)

#define EMULOGCPU(component, level, format, ...) \
	if (EMULOG_WANTS(component, level)) \
		emulog_log_cpu(component, level, format, ##__VA_ARGS__)

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
