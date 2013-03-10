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

#include "errors.h"
#include "timer.h"
#include "registers.h"
#include "interrupts.h"

int timer_fin = 0;

pthread_t timer_th;

// -----------------------------------------------------------------------
void * timer_thread(void *ptr)
{
	struct timespec ts;
	struct timespec tr;

	ts.tv_sec = 0;
	ts.tv_nsec = TIMER_PERIOD * 1000000;

	while (!timer_fin) {
		nanosleep(&ts, &tr);
		int_set(INT_TIMER);
	}
	pthread_exit(NULL);
}

// -----------------------------------------------------------------------
int timer_init()
{
	return pthread_create(&timer_th, NULL, timer_thread, NULL);
}

// -----------------------------------------------------------------------
void timer_shutdown()
{
	timer_fin = 1;
	if (timer_th) {
		pthread_join(timer_th, NULL);
	}
}

// vim: tabstop=4
