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
#include "errors.h"

struct logger *l;

uint16_t emulog_cycle_ic;
char emulog_pname[7] = "------";
FILE *emulog_f;
int emulog_enabled;
int emulog_paused;

#define EMULOG_INT_INDENT_MAX 4*8
int emulog_int_level = EMULOG_INT_INDENT_MAX;
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
int emulog_init(int paused, char *filename, char *format)
{
	emulog_f = fopen(filename, "a");
	if (!emulog_f) {
		return E_FILE_OPEN;
	}
		
	l = log_init(emulog_f, format, emulog_comp_names);

	if (!l) {
		fclose(emulog_f);
		return E_LOGGER;
	}

	emulog_enabled = 1;
	emulog_paused = paused;

	return E_OK;
}

// -----------------------------------------------------------------------
void emulog_shutdown()
{
	log_shutdown(l);
	if (emulog_f) {
		fclose(emulog_f);
	}
}

// -----------------------------------------------------------------------
void emulog_pause()
{
	atom_store(&emulog_paused, 1);
}

// -----------------------------------------------------------------------
void emulog_rec()
{
	atom_store(&emulog_paused, 0);
}

// -----------------------------------------------------------------------
int emulog_is_paused()
{
	return atom_load(&emulog_paused);
}

// -----------------------------------------------------------------------
int emulog_set_level(int component, unsigned level)
{
	return log_set_level(l, component, level);
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

	log_do(l, component, level, EMULOG_FORMAT_NONCPU "%s", " .---------------------------------------------------------");
	while (start && *start) {
		p = strchr(start, '\n');
		if (p) {
			*p = '\0';
			log_do(l, component, level, EMULOG_FORMAT_NONCPU " | %s", start);
			start = p+1;
		} else {
			log_do(l, component, level, EMULOG_FORMAT_NONCPU " | %s", start);
			start = NULL;
		}
	}
	log_do(l, component, level, EMULOG_FORMAT_NONCPU "%s", " `---------------------------------------------------------");

}

// -----------------------------------------------------------------------
int emulog_wants(int component, int level)
{
	return !emulog_paused && log_allowed(l, component, level);
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
