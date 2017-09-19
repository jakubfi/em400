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

#define _XOPEN_SOURCE 600

#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#include "io/mx_timer.h"
#include "io/mx_ev.h"

#include "log.h"
#include "cfg.h"
#include "atomic.h"

#define MX_TIMER_TICK_MSEC 500

// -----------------------------------------------------------------------
void * mx_timer_thread(void *ptr)
{
	struct mx_timer *t = ptr;

	struct timespec ts;
	unsigned clock_tick_nsec = t->timer_step_ms * 1000000;
	unsigned new_nsec;
	clock_gettime(CLOCK_REALTIME , &ts);

	while (1) {
		new_nsec = ts.tv_nsec + clock_tick_nsec;
		ts.tv_sec += new_nsec / 1000000000L;
		ts.tv_nsec = new_nsec % 1000000000L;
		if (!sem_timedwait(&(t->mx_timer_quit), &ts)) {
			break;
		}

		// HW/emulation difference here:
		// Hardware timer starts to tick after it is enabled during 'setconf'.
		// Emulated timer ticks always, but starts reporting interrupts (events)
		// after it is enabled. It doesn't really matter, because line
		// activity is not synchronized with the timer, so line timeout
		// accuracy is (-0.5+task_manager_delay) seconds anyway.

		int *enabled = &(t->timer_enabled);
		if (atom_load_acquire(enabled)) {
			struct mx_ev *ev = mx_ev_simple(MX_EV_TIMER);
			if (mx_evq_enqueue(t->evq, ev, MX_EVQ_F_WAIT) <= 0) {
				mx_ev_delete(ev);
			}
		}
	}

	pthread_exit(NULL);
}

// -----------------------------------------------------------------------
struct mx_timer * mx_timer_init(int timer_step_ms, struct mx_evq *evq)
{
	struct mx_timer *t = malloc(sizeof(struct mx_timer));
	if (!t) {
		return NULL;
	}

	t->timer_step_ms = timer_step_ms;
	sem_init(&(t->mx_timer_quit), 0, 0);
	t->evq = evq;
	atom_store_release(&(t->timer_enabled), 0);

	if (pthread_create(&(t->mx_timer_th), NULL, mx_timer_thread, t)) {
		free(t);
		return NULL;
	}

	return t;
}

// -----------------------------------------------------------------------
void mx_timer_shutdown(struct mx_timer *t)
{
	LOGID(L_MX, 1, t, "Shutting down timer");
	if (t->mx_timer_th) {
		sem_post(&(t->mx_timer_quit));
		pthread_join(t->mx_timer_th, NULL);
	}
	sem_destroy(&(t->mx_timer_quit));
	free(t);
}

// -----------------------------------------------------------------------
void mx_timer_on(struct mx_timer *t)
{
	LOGID(L_MX, 1, t, "Enabling timer");
	atom_store_release(&(t->timer_enabled), 1);
}

// -----------------------------------------------------------------------
void mx_timer_off(struct mx_timer *t)
{
	LOGID(L_MX, 1, t, "Disabling timer");
	atom_store_release(&(t->timer_enabled), 0);
}

// vim: tabstop=4 shiftwidth=4 autoindent
