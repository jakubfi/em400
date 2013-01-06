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

#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdarg.h>

#include "errors.h"
#include "debugger/log.h"

char *log_dname[] = {
	"REG",
	"MEM",
	"CPU",
	"IO",
	"INT",
	"MISC",
	NULL
};

char *log_reg_name[] = {
    "R0",
    "R1",
    "R2",
    "R3",
    "R4",
    "R5",
    "R6",
    "R7",
    "IC",
    "SR",
    "IR",
    "KB",
    "MOD",
    "MODc",
    "P",
    "ZC17"
};

char *log_int_name[] = {
	"CPU power loss",
	"memory parity",
	"no memory",
	"2nd CPU (high)",
	"ext power loss",
	"timer",
	"illegal opcode",
	"AWP div overflow",
	"AWP underflow",
	"AWP overflow",
	"AWP div/0",
	"special",
	"channel 0",
	"channel 1",
	"channel 2",
	"channel 3",
	"channel 4",
	"channel 5",
	"channel 6",
	"channel 7",
	"channel 8",
	"channel 9",
	"channel 10",
	"channel 11",
	"channel 12",
	"channel 13",
	"channel 14",
	"channel 15",
	"OPRQ",
	"2nd CPU (lower)",
	"software high",
	"software low"
};

FILE *log_f;
char *log_fname;

int log_enabled = 0;
int log_level[D_MAX] = { 0 };

// -----------------------------------------------------------------------
int log_init(const char *logf)
{
	log_fname = strdup(logf);
	log_f = fopen(log_fname, "a");
	if (!log_f) {
		return E_LOG_OPEN;
	}
	return E_OK;
}

// -----------------------------------------------------------------------
void log_shutdown()
{
	fclose(log_f);
	free(log_fname);
}

// -----------------------------------------------------------------------
int log_enable()
{
	if (!log_enabled) {
		log_f = fopen(log_fname, "a");
		if (!log_f) {
			return E_LOG_OPEN;
		} else {
			log_enabled = 1;
		}
	}
	return E_OK;
}

// -----------------------------------------------------------------------
void log_disable()
{
	if (log_enabled) {
		log_enabled = 0;
		fclose(log_f);
	}
}

// -----------------------------------------------------------------------
int log_find_domain(char *name)
{
	if (!strcasecmp(name, "all")) {
		return -1;
	}

	char **d = log_dname;
	int i = 0;
	while (*(d+i)) {
		if (!strcasecmp(name, *(d+i))) {
			return i;
		}
		i++;
	}
	return -666;
}

// -----------------------------------------------------------------------
void log_setlevel(int domain, int level)
{
	if (domain == -1) {
		for (int i=0 ; i<D_MAX ; i++) {
			log_level[i] = level;
		}
	} else {
		log_level[domain] = level;
	}
}

// -----------------------------------------------------------------------
void log_log(int domain, int level, char *format, ...)
{
	if (log_level[domain] < level) {
		return;
	}

	char now[30+1];
	struct timeval ct;

	gettimeofday(&ct, NULL);
	strftime(now, 30, "%Y-%m-%d %H:%M:%S", localtime(&ct.tv_sec));

	va_list vl;
	va_start(vl, format);
	fprintf(log_f, "%s.%-6d %4s:%-3d ", now, (int) ct.tv_usec, log_dname[domain], level);
	vfprintf(log_f, format, vl);
	fprintf(log_f, "\n");
	va_end(vl);
	fflush(log_f);
}

// vim: tabstop=4
