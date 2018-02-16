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

#ifndef MX_EV_H
#define MX_EV_H

#include "log.h"

enum mx_events {
	MX_EV_QUIT,
	MX_EV_RESET,
	MX_EV_INT_RECVD,
	MX_EV_TIMER,
	MX_EV_LINE,
	MX_EV_CMD,
	MX_EV_MAX
};

struct mx_ev {
	int active;
	unsigned cmd, line, arg;
	void *data;
};

struct mx_evt {
	struct mx_ev event[MX_EV_MAX];
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	int enabled;
	LOG_ID_DEF;
};

const char * mx_ev_name(unsigned i);

struct mx_evt * mx_evt_create();
void mx_evt_clear(struct mx_evt *queue);
void mx_evt_destroy(struct mx_evt *queue);

void mx_evt_quit(struct mx_evt *evt);
void mx_evt_timer(struct mx_evt *evt);
int mx_evt_intrecvd(struct mx_evt *evt);
int mx_evt_cmd(struct mx_evt *evt, int cmd, int log_n, uint16_t r_arg);

int mx_evt_get(struct mx_evt *evt);

void mx_evt_enable(struct mx_evt *queue);
void mx_evt_disable(struct mx_evt *queue);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
