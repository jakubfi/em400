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
#include <stdlib.h>

#include "io/mx_line.h"
#include "io/mx_irq.h"
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

static const char *mx_cond_names[] = {
	"start",
	"recv. done",
	"send done",
	"error",
	"oprq",
	"winch/xon",
	"[nonexist]",
	"timer",
	"[unknown-cond]"
};

// -----------------------------------------------------------------------
struct mx_taskset * mx_ts_create(struct mx_irqq *irqq)
{
	struct mx_taskset *ts = calloc(1, sizeof(struct mx_taskset));
	if (!ts) {
		goto cleanup;
	}

	ts->irqq = irqq;

    if (pthread_cond_init(&ts->cond, NULL)) {
        goto cleanup;
    }

	if (pthread_mutex_init(&ts->mutex, NULL)) {
		goto cleanup;
	}

	return ts;

cleanup:
	mx_ts_destroy(ts);
	return NULL;
}

// -----------------------------------------------------------------------
void mx_ts_destroy(struct mx_taskset *ts)
{
	if (!ts) return;
	pthread_mutex_destroy(&ts->mutex);
	pthread_cond_destroy(&ts->cond);
	free(ts);
}

// -----------------------------------------------------------------------
int mx_ts_line_configure(struct mx_taskset *ts, unsigned line_num, struct mx_line *line)
{
	pthread_mutex_lock(&ts->mutex);
	for (int tn=0 ; tn<MX_TASK_MAX ; tn++) {
		ts->task[tn].line[line_num].lineptr = line;
		ts->task[tn].line[line_num].line_num = line_num;
		ts->task[tn].line[line_num].proto = line->proto->task + tn;
		ts->task[tn].line[line_num].tg = ts->task + tn;
	}
	pthread_mutex_unlock(&ts->mutex);
	return 0;
}

// -----------------------------------------------------------------------
void mx_ts_lines_deconfigure(struct mx_taskset *ts)
{
	pthread_mutex_lock(&ts->mutex);
	for (int tn=0 ; tn<MX_TASK_MAX ; tn++) {
		for (int ln=0 ; ln<MX_LINE_MAX ; ln++) {
			ts->task[tn].line[ln].lineptr = NULL;
			ts->task[tn].line[ln].proto = NULL;
		}
	}
	pthread_mutex_unlock(&ts->mutex);
}

// -----------------------------------------------------------------------
const char *mx_task_name(unsigned i)
{
	if (i >= MX_TASK_MAX) {
		i = MX_TASK_MAX;
	}
	return mx_task_names[i];
}

// -----------------------------------------------------------------------
const char * mx_cond_name(unsigned i)
{
	if (i >= 8) {
		i = 8;
	}
	return mx_cond_names[i];

}
// -----------------------------------------------------------------------
int mx_task_queue(struct mx_taskset *ts, unsigned line_num, int task_num, uint16_t arg, int irq_no_line, int irq_reject)
{
	struct mx_task *task = ts->task[task_num].line + line_num;
	int ret = 0;

	LOG(L_MX, 3, "Queue task");
	
	pthread_mutex_lock(&ts->mutex);

	// if line is not configured
	if (!task->lineptr) {
		mx_irqq_enqueue(ts->irqq, irq_no_line, line_num);
		ret = -1;
		goto done;
	}

	// if there is already unconditional start scheduled or...
	// if task is waiting for a condition or...
	// if task is active for the line,
	if ((task->cond_signal & MX_SIGNAL_START) | task->cond_wait) {
		mx_irqq_enqueue(ts->irqq, irq_reject, line_num);
		ret = -2;
		goto done;
	}

	// schedule task start
	task->arg = arg;
	task->cond_signal = MX_SIGNAL_START; // task is scheduled for unconditional start
	ts->task[task_num].scheduled++;
	pthread_cond_signal(&ts->cond);

done:
	pthread_mutex_unlock(&ts->mutex);
	return ret;
}

// -----------------------------------------------------------------------
static int mx_task_is_waiting(struct mx_task *task)
{
	// if task is waiting for any condition...
	// or has just been started
	if ((task->cond_wait | MX_TASK_ACT) & task->cond_signal) {
		return 1;
	}
	return 0;
}

// -----------------------------------------------------------------------
int mx_task_get_max_condition_num(struct mx_task *task)
{
	for (unsigned i=0 ; i<8 ; i++) {
		if ((task->cond_signal & (1<<i))) {
			return i;
		}
	}

	return -1;
}

// -----------------------------------------------------------------------
static void mx_task_activate(struct mx_task *task)
{
	task->cond_wait = MX_TASK_ACT; // task is active, not waiting for anything
	task->cond_signal &= ~MX_SIGNAL_START; // task is not scheduled for unconditional start
}

// -----------------------------------------------------------------------
static struct mx_task * mx_task_top(struct mx_taskset *ts, int *cond)
{
	struct mx_task *task = NULL;
	static int cur_line_num = 0;

	// find highest priority scheduled task
	for (int tn=0 ; tn<MX_TASK_MAX ; tn++) {
		if (ts->task[tn].scheduled) {

			// find first line waiting on the task
			for (int ln=0 ; ln<MX_LINE_MAX ; ln++) {
				cur_line_num = (cur_line_num+1) % MX_LINE_MAX;
				task = ts->task[tn].line + cur_line_num;
				if (task->lineptr && mx_task_is_waiting(task)) {
					*cond = mx_task_get_max_condition_num(task);
					mx_task_activate(task);
					pthread_mutex_unlock(&ts->mutex);
					LOG(L_MX, 3,"Line %i running task %i: %s", cur_line_num, tn, mx_task_name(tn));
					return task;
				}
			}

			// task seems scheduled, but no line was waiting on that task
			ts->task[tn].scheduled = 0;
		}
	}

	return NULL;
}

// -----------------------------------------------------------------------
struct mx_task * mx_task_dequeue(struct mx_taskset *ts, int *cond)
{
	struct mx_task *task = NULL;

	pthread_mutex_lock(&ts->mutex);
	task = mx_task_top(ts, cond);
	if (!task) pthread_cond_wait(&ts->cond, &ts->mutex);
	pthread_mutex_unlock(&ts->mutex);

	return task;
}

// -----------------------------------------------------------------------
void mx_task_finalize(struct mx_taskset *ts, struct mx_task *task, int cond_wait, int irq)
{
	pthread_mutex_lock(&ts->mutex);

	task->cond_wait = cond_wait;

	// if task doesn't require any more attention, finish it
	if (cond_wait == MX_WAIT_NONE) {
		task->tg->scheduled--;
		LOG(L_MX, 3, "Line %i task is finished", task->line_num);
	}

	// If task wants to send an IRQ, do it here
	if (irq != MX_IRQ_NONE) {
		mx_irqq_enqueue(ts->irqq, irq, task->line_num);
	}

	pthread_mutex_unlock(&ts->mutex);
}

// -----------------------------------------------------------------------
void mx_task_idle_run(struct mx_taskset *ts)
{
	pthread_mutex_lock(&ts->mutex);
	pthread_cond_signal(&ts->cond);
	pthread_mutex_unlock(&ts->mutex);
}

// -----------------------------------------------------------------------
void mx_task_dma_done()
{

}

// vim: tabstop=4 shiftwidth=4 autoindent
