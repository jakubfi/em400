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

#include <inttypes.h>

#include "io/mx_line.h"
#include "io/mx_task.h"

static const char *mx_task_names[] = {
	"report status",
	"detach line",
	"OPRQ",
	"transmit",
	"abort transmission",
	"attach line",
	"[unknown-task]"
};

// -----------------------------------------------------------------------
const char *mx_task_name(unsigned i)
{
	if (i >= MX_TASK_MAX) {
		i = MX_TASK_MAX;
	}
	return mx_task_names[i];
}

// -----------------------------------------------------------------------
int mx_task_is_running(struct mx_task *task)
{
	// if there is already unconditional start scheduled or...
	// if task is waiting for a condition or...
	// if task is active for the line,
	return (task->bwar & MX_COND_START) | task->bzaw;
}

// -----------------------------------------------------------------------
void mx_task_start(struct mx_task *task, uint16_t arg)
{
	// schedule task start
	task->arg = arg;
	task->bwar = MX_COND_START; // task is scheduled for unconditional start
}

// -----------------------------------------------------------------------
int mx_task_is_waiting(struct mx_task *task)
{
	// if task is waiting for any condition...
	// or has just been started
	if ((task->bzaw | MX_COND_START) & task->bwar) {
		return 1;
	}
	return 0;
}

// -----------------------------------------------------------------------
void mx_task_activate(struct mx_task *task)
{
	task->bzaw = MX_COND_ACT; // task is active, not waiting for anything
	task->bwar &= ~MX_COND_START; // task is not scheduled for unconditional start
}

// -----------------------------------------------------------------------
unsigned mx_task_get_condition_num(struct mx_task *task)
{
	for (unsigned i=0 ; i<8 ; i++) {
		if ((task->bwar & (1<<i))) {
			return i;
		}
	}

	return 0;
}

// vim: tabstop=4 shiftwidth=4 autoindent
