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

#ifndef LOG_H
#define LOG_H

#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

enum log_domains {
	D_REG = 0,
	D_MEM,
	D_CPU,
	D_IO,
	D_INT,
	D_MISC,
	D_MAX
};

/*
Log levels:

* CPU:
	1 - step
	5 - exec (dasm+trnas)
	10 - eff. arg.
* REG:
	1 - write
	10 - read traced
	15 - read all
* MEM:
	1 - add map, load image
	5 - error accessing
	10 - write traced
	20 - read traced
	30 - write all
	40 - read all
* I/O:
	1 - command
	10 - channel
	20 - unit
	30 - data
* INT:
	10 - everything
*/

extern FILE *log_f;
extern int log_level[];
extern char *log_pname[];

extern char *log_reg_name[];
extern char *log_int_name[];

int log_init(const char *logf);
void log_shutdown();
void log_setlevel(int domain, int level);
void log_log(int domain, int level, char *format, ...);

#ifdef WITH_DEBUGGER
#define LOG(d, l, f, ...) log_log(d, l, f, ##__VA_ARGS__)
#else
#define LOG(d, l, f, ...) ;
#endif

#endif

// vim: tabstop=4

