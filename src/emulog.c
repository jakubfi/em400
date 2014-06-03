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

#include <stdlib.h>
#include <string.h>

#include "logger.h"
#include "emulog.h"

struct logger *l;
char *emulog_fname;

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

char *emulog_io_result_names[] = {
	"NO DEVICE",
	"ENGAGED",
	"OK",
	"PARITY ERROR"
};


// -----------------------------------------------------------------------
char * emulog_get_fname()
{
	return emulog_fname;
}

// -----------------------------------------------------------------------
int emulog_open(char *filename)
{
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


// vim: tabstop=4 shiftwidth=4 autoindent
