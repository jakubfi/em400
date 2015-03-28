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

enum mx_evq_events {
	MX_EV_CMD,
	MX_EV_INT_RECVD,
	MX_EV_TIMER,
	MX_EV_LINE,
	MX_EV_NONE,
};

enum mx_evq_flags {
	MX_EVQ_F_NONE	= 0,
	MX_EVQ_F_WAIT	= 1 << 0,
	MX_EVQ_F_TRY	= 1 << 1,
};

extern const char *mx_event_names[];

struct mx_ev {
	int type;
	struct mx_ev *next;
	unsigned cmd, line, arg;
	void *data;
};

struct mx_evq {
	struct mx_ev *head, *tail;
	int size;
	int maxlen;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	int state;
};

struct mx_ev * mx_ev_simple(int type);
struct mx_ev * mx_ev_cmd(unsigned cmd, unsigned line, unsigned arg);
struct mx_ev * mx_ev_line(unsigned size, void *data);
void mx_ev_delete(struct mx_ev *event);

struct mx_evq * mx_evq_create(int maxlen);
void mx_evq_clear(struct mx_evq *queue);
int mx_evq_size(struct mx_evq *queue);
void mx_evq_destroy(struct mx_evq *queue);
int mx_evq_insert(struct mx_evq *queue, struct mx_ev *event, int flags);
int mx_evq_prequeue(struct mx_evq *queue, struct mx_ev *event, int flags);
int mx_evq_enqueue(struct mx_evq *queue, struct mx_ev *event, int flags);
struct mx_ev * mx_evq_dequeue(struct mx_evq *queue, int flags);
void mx_evq_enable(struct mx_evq *queue);
void mx_evq_disable(struct mx_evq *queue);


#endif

// vim: tabstop=4 shiftwidth=4 autoindent
