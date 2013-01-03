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

enum log_parts {
	P_REG_R		= 18,
	P_REG_W		= 17,
	P_MEM_R		= 16,
	P_MEM_W		= 15,
	P_MEM_CFG	= 14,

	P_CPU_STEP	= 13,
	P_CPU_OP	= 12,
	P_CPU_EFF	= 11,
	P_CPU_x		= 10,
	P_CPU_y		= 9,

	P_IO_DECODE	= 8,
	P_IO_SPEC	= 7,
	P_IO_CHAN	= 6,
	P_IO_UNIT	= 5,
	P_IO_x		= 4,

	P_INT_SERVE	= 3,
	P_INT_SET	= 2,
	P_INT_CLEAR	= 1,

	P_MISC		= 0
};

extern char *log_pname[];

enum _log_mask {
	M_REG_R		= 1 << P_REG_R,
	M_REG_W		= 1 << P_REG_W,
	M_REG_ALL	= M_REG_R | M_REG_W,

	M_MEM_CFG	= 1 << P_MEM_CFG,
	M_MEM_R		= 1 << P_MEM_R,
	M_MEM_W		= 1 << P_MEM_W,
	M_MEM_ALL	= M_MEM_CFG | M_MEM_R | M_MEM_W,

	M_CPU_STEP	= 1 << P_CPU_STEP,
	M_CPU_OP	= 1 << P_CPU_OP,
	M_CPU_EFF	= 1 << P_CPU_EFF,
	M_CPU_x		= 1 << P_CPU_x,
	M_CPU_y		= 1 << P_CPU_y,
	M_CPU_ALL	= M_CPU_STEP | M_CPU_OP | M_CPU_EFF | M_CPU_x | M_CPU_y,

	M_IO_DECODE	= 1 << P_IO_DECODE,
	M_IO_SPEC	= 1 << P_IO_SPEC,
	M_IO_CHAN	= 1 << P_IO_CHAN,
	M_IO_UNIT	= 1 << P_IO_UNIT,
	M_IO_x		= 1 << P_IO_x,
	M_IO_ALL	= M_IO_DECODE | M_IO_SPEC | M_IO_CHAN | M_IO_UNIT | M_IO_x,

	M_INT_SERVE	= 1 << P_INT_SERVE,
	M_INT_SET	= 1 << P_INT_SET,
	M_INT_CLEAR	= 1 << P_INT_CLEAR,
	M_INT_ALL	= M_INT_SERVE | M_INT_SET | M_INT_CLEAR,

	M_MISC		= 1 << P_MISC,

	M_ALL		= 0b11111111111111111111111111111111,
	M_NONE		= 0
};

extern FILE *log_f;
extern unsigned int log_mask;

extern char *log_pname[];
extern char *log_reg_name[];
extern char *log_int_name[];

int log_init(const char *logf);
void log_shutdown();
void log_setmask(unsigned int mask);
void log_log(unsigned int part, char *format, ...);

#ifdef WITH_DEBUGGER
#define LOG(p, f, ...) log_log(p, f, ##__VA_ARGS__)
#else
#define LOG(p, f, ...) ;
#endif

#endif

// vim: tabstop=4

