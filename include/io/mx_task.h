//  Copyright (c) 2013-2015 Jakub Filipowicz <jakubf@gmail.com>
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

#include <inttypes.h>

struct mx_line;

enum mx_tasks {
	MX_TASK_UNKNOWN = -1, // em400: task for unknown MULTIX commands
	MX_TASK_MISPLCD = -2, // em400: task for misplaced MULTIX commands
	MX_TASK_STATUS	= 0,  // report status (highest priority)
	MX_TASK_DETACH	= 1,  // detach line
	MX_TASK_OPRQ	= 2,  // OPRQ
	MX_TASK_TRANSMIT= 3,  // transmit
	MX_TASK_ABORT	= 4,  // abort
	MX_TASK_ATTACH	= 5,  // attach line (lowest priority)
	MX_TASK_MAX		= 6,  // (task count)
};

enum mx_task_conditions {
	MX_COND_NONE	= 0,
	MX_COND_ACT		= 1 << 0, // active
	MX_COND_START	= 1 << 0, // reported (highest priority)
	MX_COND_RECV	= 1 << 1, // receiving done
	MX_COND_SEND	= 1 << 2, // transmitting done
	MX_COND_ERR		= 1 << 3, // error
	MX_COND_OPRQ	= 1 << 4, // OPRQ
	MX_COND_WINCH	= 1 << 5, // winchester finished or...
	MX_COND_XON		= 1 << 5, // ...XON for serial lines
	MX_COND_X		= 1 << 6, // em400: nonexisting condition, shouldn't happen
	MX_COND_TIMER	= 1 << 7, // timer (lowest priority)
};

struct mx_task {
	uint8_t bzaw;	// B.ZAWIESZENIA: /0/=BIT AKTYWNOSCI, /1-7/=BITY ZAWIESZENIA
	uint8_t bwar;	// B.WARUNKOW: /0/=<START BEZWARUN.>, /1-7/=BITY WARUNKOW
	uint16_t arg;
};

const char *mx_task_name(unsigned i);
int mx_task_is_running(struct mx_task *task);
void mx_task_start(struct mx_task *task, uint16_t arg);
int mx_task_is_waiting(struct mx_task *task);
void mx_task_activate(struct mx_task *task);
unsigned mx_task_get_condition_num(struct mx_task *task);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
