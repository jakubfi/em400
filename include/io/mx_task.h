//  Copyright (c) 2013-2014 Jakub Filipowicz <jakubf@gmail.com>
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

#ifndef MX_TASK_H
#define MX_TASK_H

#include "io/mx.h"

enum mx_tasks {
	MX_TASK_QUIT	= -2, // quit (em400)
	MX_TASK_NONE	= -1, // no task (em400)
	MX_TASK_STR		= 0, // report status
	MX_TASK_ODL		= 1, // detach line
	MX_TASK_ORQ		= 2, // OPRQ
	MX_TASK_TRA		= 3, // transmit
	MX_TASK_ABT		= 4, // abort
	MX_TASK_DOL		= 5, // attach line
	MX_TASK_MAX		= 6, // (task count)
};

enum mx_task_conditions {
	MX_COND_NONE	= 0,
	MX_COND_ACT		= 0b00000001, // active
	MX_COND_START	= 0b00000001, // reported
	MX_COND_WAODB	= 0b00000010, // receiving done
	MX_COND_WANAD	= 0b00000100, // transmitting done
	MX_COND_WAAWA	= 0b00001000, // error
	MX_COND_WAOPR	= 0b00010000, // OPRQ
	MX_COND_WAFWI	= 0b00100000, // winchester finished, XON for serial lines
	MX_COND_X		= 0b01000000, // em400: nonexisting condition, shouldn't happen
	MX_COND_WATIM	= 0b10000000, // timer 
};

struct mx_line_cond {
	int sleep;
	int condition;
};

struct mx_task_line {
	int reported;
	struct mx_line_cond line[MX_LINE_MAX];
};

struct mx_cmd_table {
	int cmd;
	int task_n;
	int intr_reject;
	int intr_noline;
};

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
