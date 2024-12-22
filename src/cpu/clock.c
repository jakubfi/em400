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
#include <stdatomic.h>

#include "cpu/clock.h"
#include "cpu/interrupts.h"

#include "log.h"

int clock_enabled = false;
pthread_t clock_th;
sem_t clock_quit;

int clock_period = 10;
int clock_int = INT_CLOCK;

// -----------------------------------------------------------------------
void * clock_thread(void *ptr)
{
	struct timespec ts;
	unsigned clock_tick_nsec = clock_period * 1000000;
	unsigned new_nsec;
	clock_gettime(CLOCK_REALTIME , &ts);

	while (1) {
		new_nsec = ts.tv_nsec + clock_tick_nsec;
		ts.tv_sec += new_nsec / 1000000000L;
		ts.tv_nsec = new_nsec % 1000000000L;
		if (!sem_timedwait(&clock_quit, &ts)) {
			break;
		}
		if (atomic_load_explicit(&clock_enabled, memory_order_acquire)) {
			int_set(atomic_load_explicit(&clock_int, memory_order_acquire));
		}
	}

	pthread_exit(NULL);
}

// -----------------------------------------------------------------------
int clock_init(int clock_period)
{
	sem_init(&clock_quit, 0, 0);
	if (pthread_create(&clock_th, NULL, clock_thread, NULL)) {
		return LOGERR("Failed to spawn clock thread.");
	}

	pthread_setname_np(clock_th, "clock");

	LOG(L_CPU, "Clock period: %i ms", clock_period);

	return E_OK;
}

// -----------------------------------------------------------------------
void clock_shutdown()
{
	LOG(L_CPU, "Shutting down clock");
	if (clock_th) {
		sem_post(&clock_quit);
		pthread_join(clock_th, NULL);
	}
	sem_destroy(&clock_quit);
}

// -----------------------------------------------------------------------
void clock_set(bool state)
{
	LOG(L_CPU, "Set clock: %s", state ? "ON" : "OFF");
	atomic_store_explicit(&clock_enabled, state, memory_order_release);
}

// -----------------------------------------------------------------------
int clock_get()
{
	return atomic_load_explicit(&clock_enabled, memory_order_acquire);
}

// -----------------------------------------------------------------------
void clock_set_int(int interrupt)
{
	atomic_store_explicit(&clock_int, interrupt, memory_order_release);
}

// vim: tabstop=4 shiftwidth=4 autoindent
