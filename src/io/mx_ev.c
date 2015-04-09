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

#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#include "io/mx_ev.h"

enum mx_evq_state {
	MX_EVQ_ENABLED,
	MX_EVQ_DISABLED,
};

enum mx_ev_add_mode {
	ADD_ENQUEUE,
	ADD_PREQUEUE,
	ADD_INSERT,
};

const char *mx_event_names[] = {
	"CMD",
	"INT_RECVD",
	"TIMER",
	"LINE",
	"NONE",
};

// -----------------------------------------------------------------------
static struct mx_ev * mx_ev(int type, unsigned cmd, unsigned line, unsigned arg, void *data)
{
	struct mx_ev *event = malloc(sizeof(struct mx_ev));

	if (!event) return NULL;

	event->type = type;
	event->cmd = cmd;
	event->line = line;
	event->arg = arg;
	event->data = data;
	event->next = NULL;

	return event;
}

// -----------------------------------------------------------------------
struct mx_ev * mx_ev_simple(int type)
{
	return mx_ev(type, 0, 0, 0, NULL);
}

// -----------------------------------------------------------------------
struct mx_ev * mx_ev_cmd(unsigned cmd, unsigned line, unsigned arg)
{
	return mx_ev(MX_EV_CMD, cmd, line, arg, NULL);
}

// -----------------------------------------------------------------------
struct mx_ev * mx_ev_line(unsigned size, void *data)
{
	return mx_ev(MX_EV_LINE, 0, 0, size, data);
}

// -----------------------------------------------------------------------
void mx_ev_delete(struct mx_ev *event)
{
	free(event->data);
	free(event);
}

// -----------------------------------------------------------------------
struct mx_evq * mx_evq_create(int maxlen)
{
	struct mx_evq *queue = malloc(sizeof(struct mx_evq));
	if (!queue) {
		goto cleanup;
	}
	queue->head = NULL;
	queue->tail = NULL;
	queue->size = 0;
	queue->maxlen = maxlen;
	queue->state = MX_EVQ_DISABLED;

	if (pthread_mutex_init(&queue->mutex, NULL)) {
		goto cleanup;
	}
	if (pthread_cond_init(&queue->cond, NULL)) {
		goto cleanup;
	}

	return queue;

cleanup:
	mx_evq_destroy(queue);
	return NULL;
}

// -----------------------------------------------------------------------
void mx_evq_clear(struct mx_evq *queue)
{
	pthread_mutex_lock(&queue->mutex);

	struct mx_ev *event = queue->head;
	struct mx_ev *t;

	while (event) {
		t = event->next;
		mx_ev_delete(event);
		event = t;
	}

	queue->size = 0;
	queue->head = NULL;
	queue->tail = NULL;

	pthread_mutex_unlock(&queue->mutex);
}

// -----------------------------------------------------------------------
int mx_evq_size(struct mx_evq *queue)
{
	pthread_mutex_lock(&queue->mutex);
	int size = queue->size;
	pthread_mutex_unlock(&queue->mutex);
	return size;
}

// -----------------------------------------------------------------------
void mx_evq_destroy(struct mx_evq *queue)
{
	if (!queue) return;
	mx_evq_disable(queue);
	mx_evq_clear(queue);
	pthread_mutex_destroy(&queue->mutex);
	pthread_cond_destroy(&queue->cond);
	free(queue);
}

// -----------------------------------------------------------------------
static int __mx_evq_queue(struct mx_evq *queue, int mode, struct mx_ev *event, int flags)
{
	if (!event) {
		return -1;
	}

	// user wants to only try, don't wait for mutex
	if (flags & MX_EVQ_F_TRY) {
		if (pthread_mutex_trylock(&queue->mutex)) {
			return -1;
		}
	// user wants to wait for mutex
	} else {
		pthread_mutex_lock(&queue->mutex);
	}

	// is queue ready?
	if (queue->state != MX_EVQ_ENABLED) {
		pthread_mutex_unlock(&queue->mutex);
		return -1;
	}

	// user wants to wait until queue frees up
	if (flags && MX_EVQ_F_WAIT) {
		while ((queue->maxlen > 0) && (queue->size >= queue->maxlen)) {
			pthread_cond_wait(&queue->cond, &queue->mutex);
		}
	// user does not want to wait
	} else {
		if ((queue->maxlen > 0) && (queue->size >= queue->maxlen)) {
			pthread_mutex_unlock(&queue->mutex);
			return -1;
		}
	}

	struct mx_ev *before, *after;

	if (mode == ADD_ENQUEUE) {
		before = NULL;
		after = queue->tail;
	} else {
		// ADD_PREQUEUE and ADD_INSERT
		before = queue->head;
		after = NULL;
		if (mode == ADD_INSERT) {
			// find a place for the new event
			while (before && (event->type > before->type)) {
				after = before;
				before = before->next;
			}
		}
	}

	// insert event
	event->next = before;
	if (after) {
		after->next = event;
	} else {
		queue->head = event;
	}
	if (!before) {
		queue->tail = event;
	}

	// update size and notify waiters
	queue->size++;
	pthread_cond_signal(&queue->cond);
	int cur_size = queue->size;

	pthread_mutex_unlock(&queue->mutex);

	return cur_size;
}

// -----------------------------------------------------------------------
int mx_evq_insert(struct mx_evq *queue, struct mx_ev *event, int flags)
{
	return __mx_evq_queue(queue, ADD_INSERT, event, flags);
}

// -----------------------------------------------------------------------
int mx_evq_prequeue(struct mx_evq *queue, struct mx_ev *event, int flags)
{
	return __mx_evq_queue(queue, ADD_PREQUEUE, event, flags);
}

// -----------------------------------------------------------------------
int mx_evq_enqueue(struct mx_evq *queue, struct mx_ev *event, int flags)
{
	return __mx_evq_queue(queue, ADD_ENQUEUE, event, flags);
}

// -----------------------------------------------------------------------
struct mx_ev * mx_evq_dequeue(struct mx_evq *queue, int flags)
{
	pthread_mutex_lock(&queue->mutex);

	while (!queue->head && (flags & MX_EVQ_F_WAIT) && (queue->state == MX_EVQ_ENABLED)) {
		pthread_cond_wait(&queue->cond, &queue->mutex);
	}

	if (queue->state != MX_EVQ_ENABLED) {
		pthread_mutex_unlock(&queue->mutex);
		return NULL;
	}

	struct mx_ev *event = queue->head;
	struct mx_ev *ret_event = NULL;

	if (event) {
		queue->head = event->next;
		if (!queue->head) {
			queue->tail = NULL;
		}
		queue->size--;
		ret_event = event;
	}

	pthread_mutex_unlock(&queue->mutex);

	return ret_event;
}

// -----------------------------------------------------------------------
void mx_evq_enable(struct mx_evq *queue)
{
	pthread_mutex_lock(&queue->mutex);
	queue->state = MX_EVQ_ENABLED;
	pthread_mutex_unlock(&queue->mutex);
}

// -----------------------------------------------------------------------
void mx_evq_disable(struct mx_evq *queue)
{
	pthread_mutex_lock(&queue->mutex);
	queue->state = MX_EVQ_DISABLED;
	pthread_cond_signal(&queue->cond);
	pthread_mutex_unlock(&queue->mutex);
}

// vim: tabstop=4 shiftwidth=4 autoindent
