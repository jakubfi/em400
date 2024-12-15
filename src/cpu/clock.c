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

#include "cpu/clock.h"
#include "cpu/interrupts.h"

#include "log.h"
#include "cfg.h"
#include "atomic.h"

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
		if (atom_load_acquire(&clock_enabled)) {
			int_set(atom_load_acquire(&clock_int));
		}
	}

	pthread_exit(NULL);
}

// -----------------------------------------------------------------------
int clock_init(em400_cfg *cfg)
{
	clock_period = cfg_getint(cfg, "cpu:clock_period", CFG_DEFAULT_CPU_CLOCK_PERIOD);

	if ((clock_period < 2) || (clock_period > 100)) {
		return LOGERR("Clock period should be between 2 and 100 miliseconds, not %i.", clock_period);
	}

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
	atom_store_release(&clock_enabled, state);
}

// -----------------------------------------------------------------------
int clock_get()
{
	return atom_load_acquire(&clock_enabled);
}

// -----------------------------------------------------------------------
void clock_set_int(int interrupt)
{
	atom_store_release(&clock_int, interrupt);
}

// vim: tabstop=4 shiftwidth=4 autoindent
