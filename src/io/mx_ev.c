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

#include "io/defs.h"
#include "io/mx_ev.h"

#include "log.h"

const char *mx_event_names[] = {
	"QUIT",
	"RESET",
	"INT_RECVD",
	"TIMER",
	"LINE",
	"CMD",
	"[invalid-event]"
};

// -----------------------------------------------------------------------
const char * mx_ev_name(unsigned i)
{
	if (i >= MX_EV_MAX) i = MX_EV_MAX;
	return mx_event_names[i];
}

// -----------------------------------------------------------------------
struct mx_evt * mx_evt_create()
{
	struct mx_evt *evt = malloc(sizeof(struct mx_evt));
	if (!evt) {
		goto cleanup;
	}

	if (pthread_mutex_init(&evt->mutex, NULL)) {
		goto cleanup;
	}
	if (pthread_cond_init(&evt->cond, NULL)) {
		goto cleanup;
	}

	mx_evt_clear(evt);

	return evt;

cleanup:
	mx_evt_destroy(evt);
	return NULL;
}

// -----------------------------------------------------------------------
void mx_evt_clear(struct mx_evt *evt)
{
	pthread_mutex_lock(&evt->mutex);

	for (int i=0 ; i<MX_EV_MAX ; i++) {
		evt->event[i].active = 0;
	}
	pthread_cond_signal(&evt->cond);

	pthread_mutex_unlock(&evt->mutex);
}

// -----------------------------------------------------------------------
void mx_evt_destroy(struct mx_evt *evt)
{
	if (!evt) return;

	mx_evt_disable(evt);
	mx_evt_clear(evt);
	pthread_mutex_destroy(&evt->mutex);
	pthread_cond_destroy(&evt->cond);

	free(evt);
}

// -----------------------------------------------------------------------
void mx_evt_quit(struct mx_evt *evt)
{
	pthread_mutex_lock(&evt->mutex);
	evt->event[MX_EV_QUIT].active = 1;
	pthread_cond_signal(&evt->cond);
	pthread_mutex_unlock(&evt->mutex);
	LOGID(L_MX, 4, evt, "Event sent: quit");
}

// -----------------------------------------------------------------------
void mx_evt_timer(struct mx_evt *evt)
{
	pthread_mutex_lock(&evt->mutex);
	evt->event[MX_EV_TIMER].active = 1;
	pthread_cond_signal(&evt->cond);
	pthread_mutex_unlock(&evt->mutex);
	LOGID(L_MX, 4, evt, "Event sent: timer");
}

// -----------------------------------------------------------------------
int mx_evt_intrecvd(struct mx_evt *evt)
{
	pthread_mutex_lock(&evt->mutex);
	evt->event[MX_EV_INT_RECVD].active = 1;
	pthread_cond_signal(&evt->cond);
	pthread_mutex_unlock(&evt->mutex);
	LOGID(L_MX, 4, evt, "Event sent: interrupt received");
	return IO_OK;
}

// -----------------------------------------------------------------------
int mx_evt_cmd(struct mx_evt *evt, int cmd, int log_n, uint16_t r_arg)
{
	if (pthread_mutex_trylock(&evt->mutex)) {
		LOGID(L_MX, 4, evt, "Event processor busy");
		return IO_EN;
	}

	if (!evt->enabled) {
		pthread_mutex_unlock(&evt->mutex);
		LOGID(L_MX, 4, evt, "Events are disabled");
		return IO_EN;
	}

	if (evt->event[MX_EV_CMD].active) {
		LOGID(L_MX, 4, evt, "Previous cmd event not processed");
		pthread_mutex_unlock(&evt->mutex);
		return IO_EN;
	}

	evt->event[MX_EV_CMD].active = 1;
	evt->event[MX_EV_CMD].cmd = cmd;
	evt->event[MX_EV_CMD].line = log_n;
	evt->event[MX_EV_CMD].arg = r_arg;
	pthread_cond_signal(&evt->cond);

	pthread_mutex_unlock(&evt->mutex);

	LOGID(L_MX, 4, evt, "Event sent: cmd: %i, line: %i, arg: 0x%04x",
		cmd,
		log_n,
		r_arg
	);

	return IO_OK;
}

// -----------------------------------------------------------------------
static int mx_evt_topevent(struct mx_evt *evt)
{
	int i;
	for (i=0 ; i<MX_EV_MAX ; i++) {
		if (evt->event[i].active) break;
	}
	LOGID(L_MX, 4, evt, "Max event: %i", i);
	return i;
}

// -----------------------------------------------------------------------
int mx_evt_get(struct mx_evt *evt)
{
	int top_event;

	pthread_mutex_lock(&evt->mutex);

	while ((top_event = mx_evt_topevent(evt)) == MX_EV_MAX) {
		pthread_cond_wait(&evt->cond, &evt->mutex);
	}

	evt->event[top_event].active = 0;

	pthread_mutex_unlock(&evt->mutex);

	return top_event;
}

// -----------------------------------------------------------------------
void mx_evt_enable(struct mx_evt *evt)
{
	LOGID(L_MX, 4, evt, "Enabling events");
	pthread_mutex_lock(&evt->mutex);
	evt->enabled = 1;
	pthread_cond_signal(&evt->cond);
	pthread_mutex_unlock(&evt->mutex);
}

// -----------------------------------------------------------------------
void mx_evt_disable(struct mx_evt *evt)
{
	LOGID(L_MX, 4, evt, "Disabling events");
	pthread_mutex_lock(&evt->mutex);
	evt->enabled = 0;
	pthread_cond_signal(&evt->cond);
	pthread_mutex_unlock(&evt->mutex);
}

// vim: tabstop=4 shiftwidth=4 autoindent
