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

#ifndef MX_H
#define MX_H

#include <pthread.h>

#include "io/mx_ev.h"
#include "io/mx_irq.h"

struct mx {
	int num;
	pthread_t main_th;
	struct mx_evq *evq;
	struct mx_irqq *irqq;

	int state;
	pthread_mutex_t state_mutex;
	pthread_cond_t state_cond;

	int reset_ack;
	pthread_mutex_t reset_ack_mutex;
	pthread_cond_t reset_ack_cond;
};

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
