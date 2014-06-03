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

#include "logger.h"

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

extern char *emulog_io_result_names[];
extern int emulog_enabled;

char *emulog_get_fname();
int emulog_open(char *filename);
int emulog_close();
int emulog_enable();
int emulog_disable();
int emulog_set_level(int component, unsigned level);
int emulog_is_enabled();
char * emulog_get_component_name(int component);
int emulog_get_component_id(char *name);
int emulog_get_level(int component);
void emulog_log(int component, int level, char *format, ...);
void emulog_splitlog(int component, int level, char *text);
int emulog_wants(int component, int level);

#define EMULOG_WANTS(component, level) ((emulog_enabled) && (emulog_wants((component), (level))))

#ifdef WITH_EMULOG
#define EMULOG(component, level, format, ...) if (EMULOG_WANTS(component, level)) emulog_log(component, level, "              |        | " format, ##__VA_ARGS__)
#define EMULOGCPU(component, level, format, ...) if (EMULOG_WANTS(component, level)) emulog_log(component, level, "%-3s %2i:0x%04x | %s%s | " format, Q?"USR":"OS", NB, cycle_ic, pn1, pn2, ##__VA_ARGS__)
#else
#define EMULOG(component, level, format, ...) ;
#define EMULOGCPU(component, level, format, ...) ;
#endif

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
