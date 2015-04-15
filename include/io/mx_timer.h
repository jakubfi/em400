//  Copyright (c) 2015 Jakub Filipowicz <jakubf@gmail.com>
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

#ifndef MX_TIMER_H
#define MX_TIMER_H

#include <pthread.h>
#include <semaphore.h>

#include "log.h"
#include "io/mx_ev.h"

struct mx_timer {
	int timer_enabled;
	int timer_step_ms;

	pthread_t mx_timer_th;
	sem_t mx_timer_quit;

	struct mx_evq *evq;

	LOG_ID_DEF;
};

struct mx_timer * mx_timer_init();
void mx_timer_shutdown(struct mx_timer *t);
void mx_timer_on(struct mx_timer *t);
void mx_timer_off(struct mx_timer *t);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
