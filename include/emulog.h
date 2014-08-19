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

#include <inttypes.h>

#include "utils.h"
#include "logger.h"
#include "atomic.h"

#ifndef EMULOG_H
#define EMULOG_H

enum emulog_components {
	L_REG = 0,
	L_MEM,
	L_CPU,
	L_OP,
	L_INT,

	L_IO,

	L_MX,
	L_PX,
	L_CHAR,
	L_CMEM,

	L_TERM,
	L_WNCH,
	L_FLOP,
	L_PNCH,
	L_PNRD,

	L_CRK5,
	L_MAX,
};

extern int emulog_enabled;

int emulog_init(int paused, char *filename, char *format);
void emulog_shutdown();
void emulog_pause();
void emulog_rec();
int emulog_is_paused();
int emulog_set_level(int component, unsigned level);
char * emulog_get_component_name(int component);
int emulog_get_component_id(char *name);
int emulog_get_level(int component);
void emulog_log(int component, int level, char *format, ...);
void emulog_splitlog(int component, int level, char *text);
int emulog_wants(int component, int level);
void emulog_set_cycle_ic(uint16_t ic);
uint16_t emulog_get_cycle_ic();
void emulog_update_pname(uint16_t *r40pname);
char * emulog_get_pname();
void emulog_exl_store(int number, int nb, int addr, int r4);
void emulog_exl_fetch(int *number, int *nb, int *addr, int *r4);
void emulog_exl_reset();
void emulog_intlevel_reset();
void emulog_intlevel_dec();
void emulog_intlevel_inc();
const char *emulog_intlevel_get_indent();

#define EMULOG_FORMAT_NONCPU "              |        | "
#define EMULOG_FORMAT_CPU "%-3s %2i:0x%04x | %s | %s"

#define EMULOG_ENABLED (emulog_enabled)
#define EMULOG_WANTS(component, level) (EMULOG_ENABLED && emulog_wants(component, level))

#define EMULOG(component, level, format, ...) \
	if (EMULOG_WANTS(component, level)) \
		emulog_log(component, level, EMULOG_FORMAT_NONCPU format, ##__VA_ARGS__)

#define EMULOGCPU(component, level, format, ...) \
	if (EMULOG_WANTS(component, level)) \
		emulog_log(component, level, EMULOG_FORMAT_CPU format, \
			Q ? "USR" : "OS", \
			NB, \
			emulog_get_cycle_ic(), \
			emulog_get_pname(), \
			emulog_intlevel_get_indent(), \
			##__VA_ARGS__)

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
