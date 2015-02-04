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

#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#include "cpu/timer.h"
#include "cpu/cpu.h"
#include "cpu/interrupts.h"
#include "log.h"

#include "cfg.h"
#include "errors.h"
#include "atomic.h"

int timer_enabled;
pthread_t timer_th;
sem_t timer_quit;

int timer_step;

// -----------------------------------------------------------------------
void * timer_thread(void *ptr)
{
	struct timespec ts;
	unsigned clock_tick_nsec = timer_step * 1000000;
	unsigned new_nsec;
	clock_gettime(CLOCK_REALTIME , &ts);

	while (1) {
		new_nsec = ts.tv_nsec + clock_tick_nsec;
		ts.tv_sec += new_nsec / 1000000000;
		ts.tv_nsec = new_nsec % 1000000000;
		if (!sem_timedwait(&timer_quit, &ts)) {
			break;
		}
		if (atom_load(&timer_enabled)) {
			if (cpu_mod_active) {
				int_set(INT_EXTRA);
			} else {
				int_set(INT_TIMER);
			}
		}
	}

	pthread_exit(NULL);
}

// -----------------------------------------------------------------------
int timer_init(struct cfg_em400 *cfg)
{
	timer_step = cfg->timer_step;

	if ((timer_step < 2) || (timer_step > 100)) {
		return E_TIMER_VALUE;
	}

	LOG(L_CPU, 1, "Timer cycle: %i ms", timer_step);

	if (cfg->timer_start) {
		LOG(L_CPU, 1, "Starting timer");
		timer_on();
	} else {
		LOG(L_CPU, 1, "Timer disabled at power-on");
		timer_off();
	}

	sem_init(&timer_quit, 0, 0);
	if (pthread_create(&timer_th, NULL, timer_thread, NULL)) {
		return E_THREAD;
	}

	return E_OK;
}

// -----------------------------------------------------------------------
void timer_shutdown()
{
	LOG(L_CPU, 1, "Shutting down timer");
	if (timer_th) {
		sem_post(&timer_quit);
		pthread_join(timer_th, NULL);
	}
	sem_destroy(&timer_quit);
}

// -----------------------------------------------------------------------
void timer_on()
{
	LOG(L_INT, 1, "Stopping timer");
	atom_store(&timer_enabled, 1);
}

// -----------------------------------------------------------------------
void timer_off()
{
	LOG(L_INT, 1, "Stopping timer");
	atom_store(&timer_enabled, 0);
}

// vim: tabstop=4 shiftwidth=4 autoindent
