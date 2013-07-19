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

#include <pthread.h>
#include <time.h>

#include "cpu/timer.h"
#include "cpu/registers.h"
#include "cpu/interrupts.h"

#include "cfg.h"
#include "errors.h"

pthread_mutex_t timer_active = PTHREAD_MUTEX_INITIALIZER;
pthread_t timer_th;

// -----------------------------------------------------------------------
void * timer_thread(void *ptr)
{
	struct timespec ts;
	int clock_tick_nsec = em400_cfg.cpu.timer_step * 1000000;
	long new_nsec;
	clock_gettime(CLOCK_REALTIME , &ts);

	while (1) {
		new_nsec = ts.tv_nsec + clock_tick_nsec;
		ts.tv_sec += new_nsec / 1000000000;
		ts.tv_nsec = new_nsec % 1000000000;
		if(!pthread_mutex_timedlock(&timer_active, &ts)) {
			pthread_mutex_unlock(&timer_active);
			break;
		}
		int_set(INT_TIMER);
	}
	pthread_exit(NULL);
}

// -----------------------------------------------------------------------
int timer_init()
{
	if (em400_cfg.cpu.timer_step == 0) {
		eprint("Timer disabled in configuration\n");
		return E_OK;
	} else {
		eprint("Starting timer: %i ms\n", em400_cfg.cpu.timer_step);
		pthread_mutex_lock(&timer_active);
		if (pthread_create(&timer_th, NULL, timer_thread, NULL)) {
			return E_THREAD;
		} else {
			return E_OK;
		}
	}
}

// -----------------------------------------------------------------------
void timer_shutdown()
{
	if (em400_cfg.cpu.timer_step) {
		eprint("Stopping timer\n");
		pthread_mutex_unlock(&timer_active);
		pthread_join(timer_th, NULL);
	}
}

// vim: tabstop=4 shiftwidth=4 autoindent
