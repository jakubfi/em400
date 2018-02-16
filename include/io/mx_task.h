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
#include <pthread.h>
#include "io/mx_irq.h"

// due to an include loop
#define MX_LINE_MAX 32

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

enum mx_task_wait_conditions {
	MX_WAIT_NONE	= 0,
	MX_TASK_ACT		= 1 << 0, // active
	MX_WAIT_RECV	= 1 << 1, // receiving done
	MX_WAIT_SEND	= 1 << 2, // transmitting done
	MX_WAIT_ERR		= 1 << 3, // error
	MX_WAIT_OPRQ	= 1 << 4, // OPRQ
	MX_WAIT_WINCH	= 1 << 5, // winchester controller is now free
	MX_WAIT_XON		= 1 << 5, // XON for serial lines
	// bit 6 is unused
	MX_WAIT_TIMER	= 1 << 7, // timer
};

enum mx_task_signal_conditions {
	MX_SIGNAL_NONE	= 0,
	MX_SIGNAL_START	= 1 << 0, // task start (highest priority)
	MX_SIGNAL_RECV	= 1 << 1, // receiving done
	MX_SIGNAL_SEND	= 1 << 2, // transmitting done
	MX_SIGNAL_ERR	= 1 << 3, // error
	MX_SIGNAL_OPRQ	= 1 << 4, // OPRQ
	MX_SIGNAL_WINCH	= 1 << 5, // winchester: HDD controller is now free
	MX_SIGNAL_XON	= 1 << 5, // serial lines: XON
	// bit 6 is unused
	MX_SIGNAL_TIMER	= 1 << 7, // timer (lowest priority)
};

struct mx_task {
	struct mx_task_group * tg; // task group this task belongs to
	const struct mx_proto_task *proto; // protocol for this particular on this particular logical line
	struct mx_line *lineptr; // points to a physical line
	unsigned line_num; // logical line number
	uint8_t cond_wait;	// (bzaw) conditions task is waiting for (set by the task) bit 0: task is currently active, bits 7-1: wait conditions
	uint8_t cond_signal;// (bwar) conditions signalled for the task (set elswhere)
	uint16_t arg;
};

struct mx_task_group {
	int scheduled;
	struct mx_task line[MX_LINE_MAX]; // these are logical lines
};

struct mx_taskset {
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	struct mx_irqq *irqq;
	struct mx_task_group task[MX_TASK_MAX];
};

struct mx_taskset * mx_ts_create(struct mx_irqq *irqq);
void mx_ts_destroy(struct mx_taskset *ts);
int mx_ts_line_configure(struct mx_taskset *ts, unsigned line_num, struct mx_line *line);
void mx_ts_lines_deconfigure(struct mx_taskset *ts);
const char * mx_task_name(unsigned i);
const char * mx_cond_name(unsigned i);
unsigned mx_task_get_condition_num(struct mx_task *task);
int mx_task_queue(struct mx_taskset *ts, unsigned line_num, int task_num, uint16_t arg, int irq_no_line, int irq_reject);
struct mx_task * mx_task_dequeue(struct mx_taskset *ts, int *cond);
void mx_task_finalize(struct mx_taskset *ts, struct mx_task *task, int cond_wait, int irq);
void mx_task_idle_run(struct mx_taskset *ts);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
