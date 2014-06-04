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
#include <stdlib.h>
#include <string.h>

#include "logger.h"
#include "emulog.h"

struct logger *l;
char *emulog_fname;

int emulog_enabled = 0;
uint16_t emulog_cycle_ic;
char emulog_pname[7] = "------";

#define EMULOG_INT_INDENT_MAX 4*8
int emulog_int_level;
const char *emulog_int_indent = "--> --> --> --> --> --> --> --> ";

int emulog_exl_number = -1;
int emulog_exl_nb;
int emulog_exl_addr;
int emulog_exl_r4;

const char *emulog_comp_names[] = {
	"REG",
	"MEM",
	"CPU",
	"OP",
	"INT",
	"IO",
	"MX",
	"PX",
	"CCHR",
	"CMEM",
	"TERM",
	"WNCH",
	"FLOP",
	"PNCH",
	"PNRD",
	"CRK5",
	NULL
};

// -----------------------------------------------------------------------
char * emulog_get_fname()
{
	return emulog_fname;
}

// -----------------------------------------------------------------------
int emulog_open(char *filename)
{
	if (!emulog_enabled) return 0;
	emulog_close();

	FILE *f = fopen(filename, "a");
	l = log_init(f, "%t | %4c | %3l | %m", emulog_comp_names);

	if (!l) {
		return -1;
	} else {
		emulog_fname = strdup(filename);
		return 0;
	}
}

// -----------------------------------------------------------------------
int emulog_close(char *filename)
{
	if (!l) {
		return -1;
	}

	log_shutdown(l);
	free(emulog_fname);
	emulog_fname = NULL;

	return 0;
}

// -----------------------------------------------------------------------
int emulog_enable()
{
	return log_on(l);
}

// -----------------------------------------------------------------------
int emulog_disable()
{
	return log_off(l);
}

// -----------------------------------------------------------------------
int emulog_set_level(int component, unsigned level)
{
	return log_set_level(l, component, level);
}

// -----------------------------------------------------------------------
int emulog_is_enabled()
{
	return log_is_enabled(l);
}

// -----------------------------------------------------------------------
char * emulog_get_component_name(int component)
{
	return log_get_component_name(l, component);
}

// -----------------------------------------------------------------------
int emulog_get_level(int component)
{
	return log_get_level(l, component);
}

// -----------------------------------------------------------------------
int emulog_get_component_id(char *name)
{
	return log_get_component_id(l, name);
}

// -----------------------------------------------------------------------
void emulog_log(int component, int level, char *msgfmt, ...)
{
	if (!log_allowed(l, component, level)) {
		return;
	}
	
	va_list vl;
	va_start(vl, msgfmt);
	log_vdo(l, component, level, msgfmt, vl);
	va_end(vl);
}

// -----------------------------------------------------------------------
void emulog_splitlog(int component, int level, char *text)
{
	char *p;
	char *start = text;

	if (!log_allowed(l, component, level)) {
		return;
	}

	EMULOG(component, level, "%s", " ,---------------------------------------------------------");
	while (start && *start) {
		p = strchr(start, '\n');
		if (p) {
			*p = '\0';
			EMULOG(component, level, " | %s", start);
			start = p+1;
		} else {
			EMULOG(component, level, " | %s", start);
			start = NULL;
		}
	}
	EMULOG(component, level, "%s", " `---------------------------------------------------------");
}

// -----------------------------------------------------------------------
int emulog_wants(int component, int level)
{
	return log_allowed(l, component, level);
}

// -----------------------------------------------------------------------
void emulog_set_cycle_ic(uint16_t ic)
{
	emulog_cycle_ic = ic;
}

// -----------------------------------------------------------------------
uint16_t emulog_get_cycle_ic()
{
	return emulog_cycle_ic;
}

// -----------------------------------------------------------------------
void emulog_update_pname(uint16_t *r40pname)
{
	char *n1 = int2r40(r40pname[0]);
	char *n2 = int2r40(r40pname[1]);
	snprintf(emulog_pname, 7, "%s%s", n1, n2);
	free(n1);
	free(n2);
}

// -----------------------------------------------------------------------
char * emulog_get_pname()
{
	return emulog_pname;
}

// -----------------------------------------------------------------------
void emulog_exl_store(int number, int nb, int addr, int r4)
{
	emulog_exl_number = number;
	emulog_exl_nb = nb;
	emulog_exl_addr = addr;
	emulog_exl_r4 = r4;
}

// -----------------------------------------------------------------------
void emulog_exl_fetch(int *number, int *nb, int *addr, int *r4)
{
	*number = emulog_exl_number;
	*nb = emulog_exl_nb;
	*addr = emulog_exl_addr;
	*r4 = emulog_exl_r4;
}

// -----------------------------------------------------------------------
void emulog_exl_reset()
{
	emulog_exl_number = -1;
}

// -----------------------------------------------------------------------
void emulog_intlevel_reset()
{
	emulog_int_level = EMULOG_INT_INDENT_MAX;
}

// -----------------------------------------------------------------------
void emulog_intlevel_dec()
{
	emulog_int_level += 4;
}

// -----------------------------------------------------------------------
void emulog_intlevel_inc()
{
	emulog_int_level -= 4;
}

// -----------------------------------------------------------------------
const char *emulog_intlevel_get_indent()
{
	return emulog_int_indent + emulog_int_level;
}

// vim: tabstop=4 shiftwidth=4 autoindent
