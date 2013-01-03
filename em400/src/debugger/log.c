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
#include <stdarg.h>

#include "errors.h"
#include "debugger/log.h"

char *log_pname[] = {
	"MISC",
	"INT:CLEAR",
	"INT:SET",
	"INT:SERVE",
	"IO:?",
	"IO:UNIT",
	"IO:CHAN",
	"IO:SPEC",
	"IO:DECODE",
	"CPU:?",
	"CPU:?",
	"CPU:EFF",
	"CPU:OP",
	"CPU:STEP",
	"MEM:CFG",
	"MEM:W",
	"MEM:R",
	"REG:W",
	"REG:R",
	"?"
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

unsigned int log_mask = M_NONE;

// -----------------------------------------------------------------------
int log_init(const char *logf)
{
	log_f = fopen(logf, "a");
	if (!log_f) {
		return E_LOG_OPEN;
	}
	return E_OK;
}

// -----------------------------------------------------------------------
void log_shutdown()
{
	fclose(log_f);
}

// -----------------------------------------------------------------------
void log_setmask(unsigned int mask)
{
	log_mask = mask;
}

// -----------------------------------------------------------------------
void log_log(unsigned int part, char *format, ...)
{
	if (!(log_mask & (1 << part))) {
		return;
	}

	char now[30+1];
	struct timeval ct;

	gettimeofday(&ct, NULL);
	strftime(now, 30, "%Y-%m-%d %H:%M:%S", localtime(&ct.tv_sec));

	va_list vl;
	va_start(vl, format);
	fprintf(log_f, "%s.%-6d %-10s ", now, (int) ct.tv_usec, log_pname[part]);
	vfprintf(log_f, format, vl);
	fprintf(log_f, "\n");
	va_end(vl);
	fflush(log_f);
}

// vim: tabstop=4
