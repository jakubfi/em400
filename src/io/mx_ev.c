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

#include <stdlib.h>
#include <pthread.h>

#include "io/mx_ev.h"

#include "log.h"

const char *mx_event_names[] = {
	"CMD",
	"INT_RECVD",
	"TIMER",
	"LINE",
	"NONE",
	"[invalid-event]"
};

// -----------------------------------------------------------------------
const char * mx_ev_name(unsigned i)
{
	if (i >= MX_EV_MAX) i = MX_EV_MAX;
	return mx_event_names[i];
}

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
	queue->enabled = 0;

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
int mx_evq_enqueue(struct mx_evq *queue, struct mx_ev *event, int flags)
{
	if (!event) {
		return -1;
	}

	// user wants to only try, don't wait for mutex
	if (flags & MX_EVQ_F_TRY) {
		if (pthread_mutex_trylock(&queue->mutex)) {
			LOGID(L_MX, 4, queue, "Event queue busy");
			return -1;
		}
	// user wants to wait for mutex
	} else {
		pthread_mutex_lock(&queue->mutex);
	}

	// is queue ready?
	if (!queue->enabled) {
		pthread_mutex_unlock(&queue->mutex);
		LOGID(L_MX, 4, queue, "Event queue disabled when trying to enqueue");
		return -1;
	}

	// user wants to wait until queue frees up
	if (flags && MX_EVQ_F_WAIT) {
		while ((queue->maxlen > 0) && (queue->size >= queue->maxlen)) {
			LOGID(L_MX, 4, queue, "Enqueue waiting for the event queue to free up (now %i elements)", queue->size);
			pthread_cond_wait(&queue->cond, &queue->mutex);
		}
	// user does not want to wait
	} else {
		if ((queue->maxlen > 0) && (queue->size >= queue->maxlen)) {
			LOGID(L_MX, 4, queue, "Event queue full at enqueue (%i elements)", queue->size);
			pthread_mutex_unlock(&queue->mutex);
			return -1;
		}
	}

	// insert event
	event->next = NULL;
	if (queue->tail) {
		queue->tail->next = event;
	} else {
		queue->head = event;
	}
	queue->tail = event;

	// update size
	int cur_size = ++(queue->size);

	// notify waiters
	pthread_cond_signal(&queue->cond);

	pthread_mutex_unlock(&queue->mutex);

	LOGID(L_MX, 4, queue, "Added event %i: %s (cmd: %i, line: %i, arg: 0x%04x), %i waiting",
		event->type,
		mx_ev_name(event->type),
		event->cmd,
		event->line,
		event->arg,
		cur_size
	);

	return cur_size;
}

// -----------------------------------------------------------------------
int mx_evq_wait(struct mx_evq *queue)
{
	pthread_mutex_lock(&queue->mutex);

	while (!queue->head && queue->enabled) {
		LOGID(L_MX, 3, queue, "Waiting for event");
		pthread_cond_wait(&queue->cond, &queue->mutex);
	}

	if (!queue->enabled) {
		pthread_mutex_unlock(&queue->mutex);
		return -1;
	}

	pthread_mutex_unlock(&queue->mutex);

	return 0;
}

// -----------------------------------------------------------------------
struct mx_ev * mx_evq_dequeue(struct mx_evq *queue)
{
	pthread_mutex_lock(&queue->mutex);

	struct mx_ev *event = queue->head;
	struct mx_ev *ret_event = NULL;

	if (queue->enabled && event) {
		queue->head = event->next;
		if (!queue->head) {
			queue->tail = NULL;
		}
		queue->size--;
		ret_event = event;
	}

	int cur_size = queue->size;

	pthread_mutex_unlock(&queue->mutex);

	if (ret_event) {
		LOGID(L_MX, 2, queue, "Got event %i: %s (cmd: %i, line: %i, arg: 0x%04x), %i waiting",
			ret_event->type,
			mx_ev_name(ret_event->type),
			ret_event->cmd,
			ret_event->line,
			ret_event->arg,
			cur_size
		);
	} else {
		LOGID(L_MX, 3, queue, "No event");
	}

	return ret_event;
}

// -----------------------------------------------------------------------
void mx_evq_enable(struct mx_evq *queue)
{
	LOGID(L_MX, 4, queue, "Enabling event queue");
	pthread_mutex_lock(&queue->mutex);
	queue->enabled = 1;
	pthread_mutex_unlock(&queue->mutex);
}

// -----------------------------------------------------------------------
void mx_evq_disable(struct mx_evq *queue)
{
	LOGID(L_MX, 4, queue, "Disabling event queue");
	pthread_mutex_lock(&queue->mutex);
	queue->enabled = 0;
	pthread_cond_signal(&queue->cond);
	pthread_mutex_unlock(&queue->mutex);
}

// vim: tabstop=4 shiftwidth=4 autoindent
