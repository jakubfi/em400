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

#ifndef DEBUGGER_LOG_H
#define DEBUGGER_LOG_H

#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

enum log_domains {
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

	L_OS,
	L_MAX,
};

/*
	Log levels:

	1  = basic io, interrupts, memory mappings, memory errors, load memory image, HLT
	10 = basic cpu cycle, ineffective instructions, more detailed io (channel level, unit level)
	20 = more cpu cycle, + memory writes and reads
	30 = more cpu cycle, register writes and reads
	50 = decoded I/O control fields
	100 = untraced register reads, untraced memory reads and writes, timer interrupt
*/

extern int log_enabled;
extern FILE *log_f;
extern char *log_fname;
extern int log_level[];
extern char *log_dname[];

extern char *log_reg_name[];
extern char *log_int_name[];
extern char *log_io_result[];

#define LOG_INT_INDENT_MAX 32
extern int log_int_level;

int log_init(const char *logf);
void log_shutdown();
int log_enable();
void log_disable();
int log_find_domain(char *name);
void log_setlevel(int domain, int level);
void log_log(int domain, int level, char *format, ...);
void log_splitlog(int domain, int level, char *text);

#ifdef WITH_DEBUGGER
#define LOG(d, l, f, ...) if (log_enabled) log_log(d, l, f, ##__VA_ARGS__)
#else
#define LOG(d, l, f, ...) ;
#endif

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
