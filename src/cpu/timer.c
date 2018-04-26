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
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <semaphore.h>
#include <time.h>

#include "cpu/timer.h"
#include "cpu/interrupts.h"

#include "log.h"
#include "cfg.h"
#include "atomic.h"

int timer_enabled;
pthread_t timer_th;
sem_t timer_quit;

int timer_step = 10;
int timer_int = INT_TIMER;

// -----------------------------------------------------------------------
void * timer_thread(void *ptr)
{
	struct timespec ts;
	unsigned clock_tick_nsec = timer_step * 1000000;
	unsigned new_nsec;
	clock_gettime(CLOCK_REALTIME , &ts);

	while (1) {
		new_nsec = ts.tv_nsec + clock_tick_nsec;
		ts.tv_sec += new_nsec / 1000000000L;
		ts.tv_nsec = new_nsec % 1000000000L;
		if (!sem_timedwait(&timer_quit, &ts)) {
			break;
		}
		if (atom_load_acquire(&timer_enabled)) {
			int_set(atom_load_acquire(&timer_int));
		}
	}

	pthread_exit(NULL);
}

// -----------------------------------------------------------------------
int timer_init(struct cfg_em400 *cfg)
{
	timer_step = cfg->timer_step;

	if ((timer_step < 2) || (timer_step > 100)) {
		return LOGERR("Timer step should be 2-100 miliseconds, not %i.", timer_step);
	}

	LOG(L_CPU, "Timer cycle: %i ms", timer_step);

	if (cfg->timer_start) {
		timer_on();
	} else {
		timer_off();
	}

	sem_init(&timer_quit, 0, 0);
	if (pthread_create(&timer_th, NULL, timer_thread, NULL)) {
		return LOGERR("Failed to spawn timer thread.");
	}

	pthread_setname_np(timer_th, "timer");

	return E_OK;
}

// -----------------------------------------------------------------------
void timer_shutdown()
{
	LOG(L_CPU, "Shutting down timer");
	if (timer_th) {
		sem_post(&timer_quit);
		pthread_join(timer_th, NULL);
	}
	sem_destroy(&timer_quit);
}

// -----------------------------------------------------------------------
void timer_on()
{
	LOG(L_CPU, "Starting timer");
	atom_store_release(&timer_enabled, 1);
}

// -----------------------------------------------------------------------
void timer_off()
{
	LOG(L_CPU, "Stopping timer");
	atom_store_release(&timer_enabled, 0);
}

// -----------------------------------------------------------------------
int timer_get_state()
{
	return atom_load_acquire(&timer_enabled);
}

// -----------------------------------------------------------------------
void timer_set_int(int interrupt)
{
	atom_store_release(&timer_int, interrupt);
}

// vim: tabstop=4 shiftwidth=4 autoindent
