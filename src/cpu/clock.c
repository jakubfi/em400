//  Copyright (c) 2012-2024 Jakub Filipowicz <jakubf@gmail.com>
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

#include <time.h>
#include <stdatomic.h>
#include <pthread.h>

#include "cpu/clock.h"
#include "cpu/interrupts.h"
#include "cp/cp.h"
#include "utils/compat_time.h"

#include "log.h"

#define CLOCK_MAX_BACKLOG_TICKS 2

static pthread_t clock_th;
static atomic_bool clock_quit;
static bool clock_initialized;

static unsigned clock_period;
static atomic_uint clock_int;

// -----------------------------------------------------------------------
static void * clock_thread(void *ptr)
{
	struct timespec ts;
	long clock_tick_nsec = clock_period * 1000000L;
	long new_nsec;
	clock_gettime(CLOCK_MONOTONIC, &ts);

	while (1) {
		new_nsec = ts.tv_nsec + clock_tick_nsec;
		ts.tv_sec += new_nsec / 1000000000L;
		ts.tv_nsec = new_nsec % 1000000000L;

		compat_sleep_until(&ts);
		if (atomic_load_explicit(&clock_quit, memory_order_relaxed)) {
			break;
		}

		if (cp_clock_get()) {
			int_set(atomic_load_explicit(&clock_int, memory_order_relaxed));
		}

		// A backlog deeper than CLOCK_MAX_BACKLOG_TICKS discards missed ticks
		// They coalesce into one INT_CLOCK anyway, catching up only spins the thread.
		struct timespec now;
		clock_gettime(CLOCK_MONOTONIC, &now);
		long late_ns = (now.tv_sec - ts.tv_sec) * 1000000000L + (now.tv_nsec - ts.tv_nsec);
		if (late_ns > CLOCK_MAX_BACKLOG_TICKS * clock_tick_nsec) {
			ts = now;
		}
	}

	return NULL;
}

// -----------------------------------------------------------------------
int clock_init(unsigned period_ms)
{
	if (clock_initialized) {
		return LOGERR("CPU timer (clock) already initialized");
	}

	if ((period_ms != 2) && (period_ms != 4) && (period_ms != 8) && (period_ms != 10) && (period_ms != 20)) {
		return LOGERR("Valid CPU timer (clock) periods are: 2, 4, 8, 10, 20 ms, not %i", period_ms);
	}

	clock_period = period_ms;
	atomic_store_explicit(&clock_int, INT_CLOCK, memory_order_relaxed);
	atomic_store_explicit(&clock_quit, false, memory_order_relaxed);

	if (pthread_create(&clock_th, NULL, clock_thread, NULL)) {
		return LOGERR("Failed to spawn CPU timer (clock) thread.");
	}
	if (pthread_setname_np(clock_th, "clock")) {
		LOG(L_CPU, "Failed to set CPU timer (clock) thread name.");
	}

	clock_initialized = true;
	LOG(L_CPU, "CPU timer (clock) period: %i ms", period_ms);

	return E_OK;
}

// -----------------------------------------------------------------------
void clock_shutdown()
{
	LOG(L_CPU, "Shutting down CPU timer (clock)");

	if (!clock_initialized) {
		return;
	}

	atomic_store_explicit(&clock_quit, true, memory_order_relaxed);
	pthread_join(clock_th, NULL);
	clock_initialized = false;
}

// -----------------------------------------------------------------------
void clock_set_int(unsigned interrupt)
{
	LOG(L_CPU, "Set CPU timer (clock) interrupt: %d", interrupt);
	atomic_store_explicit(&clock_int, interrupt, memory_order_relaxed);
}

// vim: tabstop=4 shiftwidth=4 autoindent
