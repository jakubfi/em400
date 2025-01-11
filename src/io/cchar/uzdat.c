//  Copyright (c) 2013-2024 Jakub Filipowicz <jakubf@gmail.com>
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
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdlib.h>
#include <strings.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <pthread.h>
#include <ctype.h>

#include <stdatomic.h>
#include <uv.h>
#include <pthread.h>

#include "em400.h"
#include "io/defs.h"
#include "io/cchar/uzdat.h"
#include "io/cchar/terminal.h"

#include "log.h"
#include "cfg.h"

uv_loop_t *ioloop;

static void * uzdat_evloop(void *ptr);
static void uzdat_on_async_quit(uv_async_t *handle);
static void uzdat_on_async_disconnect(uv_async_t *handle);
static void uzdat_on_async_reset(uv_async_t *handle);
static void uzdat_on_async_write(uv_async_t *handle);
void uzdat_on_data_received(uzdat_t *uzdat, char data);
void uzdat_on_data_sent(uzdat_t *uzdat);

// -----------------------------------------------------------------------
#define __LOG_DATA(fmt, str, data) \
	if (isprint(data)) LOG(L_UZDAT, fmt "'%c'", str, data); \
	else LOG(L_UZDAT, fmt "#%02x", str, data);

// -----------------------------------------------------------------------
static int uzdat_setup(uzdat_t *uzdat)
{
	// loop related, global
	uv_async_init(ioloop, &uzdat->async_quit, uzdat_on_async_quit);
	uv_handle_set_data((uv_handle_t*) &uzdat->async_quit, uzdat);

	uv_async_init(ioloop, &uzdat->async_disconnect, uzdat_on_async_disconnect);
	uv_handle_set_data((uv_handle_t*) &uzdat->async_disconnect, uzdat);

	uv_async_init(ioloop, &uzdat->async_reset, uzdat_on_async_reset);
	uv_handle_set_data((uv_handle_t*) &uzdat->async_reset, uzdat);

	uv_async_init(ioloop, &uzdat->async_write, uzdat_on_async_write);
	uv_handle_set_data((uv_handle_t*) &uzdat->async_write, uzdat);

	uv_timer_init(ioloop, &uzdat->timer_dir_switch);
	uv_handle_set_data((uv_handle_t*) &uzdat->timer_dir_switch, uzdat);

	return 0;
}

// -----------------------------------------------------------------------
struct cchar_unit_proto_t * uzdat_create(em400_cfg *cfg, int ch_num, int dev_num)
{
	uzdat_t *uzdat = (uzdat_t*) calloc(1, sizeof(uzdat_t));
	if (!uzdat) {
		LOGERR("Failed to allocate memory for terminal: %i.%i", ch_num, dev_num);
		return NULL;
	}

	unsigned speed = cfg_fgetint(cfg, "dev%i.%i:speed", ch_num, dev_num);
	unsigned port = cfg_fgetint(cfg, "dev%i.%i:port", ch_num, dev_num);

	if (!port || !speed) {
		LOGERR("TCP terminal needs port and emulated speed to be set.");
		goto fail;
	}

	ioloop = uv_default_loop();

	uzdat->buf_rd = -1;
	uzdat->terminal = terminal_create(uzdat, (void*) uzdat_on_data_received, (void*) uzdat_on_data_sent, port, speed);

	if (!uzdat->terminal) {
		LOGERR("Failed to create terminal");
		goto fail;
	}

	LOG(L_UZDAT, "Creating terminal: speed: %i, port: %i", speed, port);


	uzdat->state = UZDAT_STATE_IDLE;

	if (uzdat_setup(uzdat)) goto fail;

	if (pthread_create(&uzdat->thread, NULL, uzdat_evloop, uzdat)) {
		LOGERR("Failed to spawn main I/O tester thread.");
		goto fail;
	}

	pthread_setname_np(uzdat->thread, "UVTERM");

	return (struct cchar_unit_proto_t *) uzdat;

fail:
	uzdat_shutdown((struct cchar_unit_proto_t*) uzdat);
	return NULL;
}

// -----------------------------------------------------------------------
void uzdat_on_data_received(uzdat_t *uzdat, char data)
{
	int interrupt;

	pthread_mutex_lock(&uzdat->mutex);
	if (uzdat->buf_rd >= 0) {
		interrupt = UZDAT_INT_TOO_SLOW;
	} else {
		interrupt = UZDAT_INT_READY;
	}
	uzdat->buf_rd = data;
	pthread_mutex_unlock(&uzdat->mutex);

	atomic_store_explicit(&uzdat->intspec, interrupt, memory_order_release);
	cchar_int_trigger(uzdat->proto.chan);
}

// -----------------------------------------------------------------------
void uzdat_on_data_sent(uzdat_t *uzdat)
{
	LOG(L_UZDAT, "WRITE complete");

	pthread_mutex_lock(&uzdat->mutex);
	uzdat->state = UZDAT_STATE_WRITE_RDY;
	atomic_store_explicit(&uzdat->intspec, UZDAT_INT_READY, memory_order_release);
	pthread_mutex_unlock(&uzdat->mutex);

	cchar_int_trigger(uzdat->proto.chan);
}

// -----------------------------------------------------------------------
static void uzdat_on_async_quit(uv_async_t *handle)
{
	LOG(L_UZDAT, "QUIT received");
	uv_stop(ioloop);
}

// -----------------------------------------------------------------------
static void uzdat_on_dir_switch_timeout(uv_timer_t *handle)
{
	uzdat_t *uzdat = (uzdat_t*) handle->data;

	pthread_mutex_lock(&uzdat->mutex);
	uzdat->state = UZDAT_STATE_WRITE_RDY;
	atomic_store_explicit(&uzdat->intspec, UZDAT_INT_READY, memory_order_release);
	pthread_mutex_unlock(&uzdat->mutex);

	cchar_int_trigger(uzdat->proto.chan);
}

// -----------------------------------------------------------------------
static void uzdat_on_async_disconnect(uv_async_t *handle)
{
	uzdat_t *uzdat = (uzdat_t*) handle->data;

	pthread_mutex_lock(&uzdat->mutex);
	uzdat->state = UZDAT_STATE_IDLE;
	uzdat->buf_rd = -1;
	pthread_mutex_unlock(&uzdat->mutex);

	uzdat->terminal->reset((void *)uzdat->terminal);
}

// -----------------------------------------------------------------------
static void uzdat_on_async_reset(uv_async_t *handle)
{
	uzdat_t *uzdat = (uzdat_t*) handle->data;

	pthread_mutex_lock(&uzdat->mutex);
	uzdat->state = UZDAT_STATE_IDLE;
	uzdat->buf_rd = -1;
	pthread_mutex_unlock(&uzdat->mutex);

	uzdat->terminal->reset((void *)uzdat->terminal);
}

// -----------------------------------------------------------------------
static void uzdat_on_async_write(uv_async_t *handle)
{
	LOG(L_UZDAT, "WRITE received");
	uzdat_t *uzdat = (uzdat_t*) handle->data;

	pthread_mutex_lock(&uzdat->mutex);
	int state = uzdat->state;
	char data = uzdat->buf_wr;
	pthread_mutex_unlock(&uzdat->mutex);

	switch (state) {
		case UZDAT_STATE_WRITING:
			int res = uzdat->terminal->write(uzdat->terminal, data);
			if (res == 0) {
				__LOG_DATA("%s: ", "WRITE to client", data)
			} else {
				LOG(L_UZDAT, "No client connected, write lost");
			}
			break;
		case UZDAT_STATE_IDLE:
		case UZDAT_STATE_READ:
			LOG(L_UZDAT, "Switching to WRITE mode");
			uv_timer_start(&uzdat->timer_dir_switch, uzdat_on_dir_switch_timeout, 1, 0);
			break;
		default:
			LOG(L_UZDAT, "Previous WRITE active");
			break;
	}
}

// -----------------------------------------------------------------------
static void * uzdat_evloop(void *ptr)
{
	LOG(L_UZDAT, "Starting UV loop");
	uv_run(ioloop, UV_RUN_DEFAULT);
	LOG(L_UZDAT, "Exited UV loop");

	uv_loop_close(ioloop);

	pthread_exit(NULL);
}

// -----------------------------------------------------------------------
void uzdat_shutdown(struct cchar_unit_proto_t *unit)
{
	LOG(L_UZDAT, "UV Terminal shutting down");
	uzdat_t *uzdat = (uzdat_t*) unit;
	if (!uzdat) return;

	uv_async_send(&uzdat->async_quit);
	pthread_join(uzdat->thread, NULL);
	free(uzdat);
}

// -----------------------------------------------------------------------
void uzdat_reset(struct cchar_unit_proto_t *unit)
{
	LOG(L_UZDAT, "Command: RESET");
	uzdat_t *uzdat = (uzdat_t*) unit;
	uv_async_send(&uzdat->async_reset);
}

// -----------------------------------------------------------------------
int uzdat_intspec(struct cchar_unit_proto_t *unit)
{
	LOG(L_UZDAT, "Command: INTSPEC");
	uzdat_t *uzdat = (uzdat_t*) unit;

	int spec = atomic_load_explicit(&uzdat->intspec, memory_order_acquire);
	atomic_store_explicit(&uzdat->intspec, UZDAT_INT_OUTDATED, memory_order_release);

	return spec;
}

// -----------------------------------------------------------------------
bool uzdat_has_interrupt(struct cchar_unit_proto_t *unit)
{
	uzdat_t *uzdat = (uzdat_t*) unit;
	return atomic_load_explicit(&uzdat->intspec, memory_order_acquire) ? true : false;
}

// -----------------------------------------------------------------------
static int uzdat_read(uzdat_t *uzdat, uint16_t *r_arg)
{
	int res;
	LOG(L_UZDAT, "Command: READ");

	pthread_mutex_lock(&uzdat->mutex);
	int data = uzdat->buf_rd & 0xff;
	if (uzdat->buf_rd >= 0) {
		*r_arg = (*r_arg & 0xff00) | (data &0xff);
		uzdat->buf_rd = -1;
		res = IO_OK;
	} else if (uzdat->state != UZDAT_STATE_WRITING) {
		uzdat->state = UZDAT_STATE_READ;
		res = IO_EN;
	} else {
		res = IO_EN;
	}
	pthread_mutex_unlock(&uzdat->mutex);

	if (res == IO_OK) {
		__LOG_DATA("%s: ", "READ ready, received: ", data & 0xff)
	} else {
		LOG(L_UZDAT, "Buffer empty, nothing to read");
	}

	return res;
}

// -----------------------------------------------------------------------
static int uzdat_write(uzdat_t *uzdat, uint16_t *r_arg)
{
	static const char *log_msgs[] = {
		"transmitter ready, sending",
		"transmitter busy, not sending",
		"transmitter not ready, not sending",
	};
	const char *log_msg = NULL;
	bool wakeup_transmitter = false;
	int res = IO_EN;

	char data = *r_arg & 0xff;

	LOG(L_UZDAT, "Command: WRITE");

	pthread_mutex_lock(&uzdat->mutex);
	if (uzdat->state == UZDAT_STATE_WRITE_RDY) {
		uzdat->buf_wr = data;
		uzdat->state = UZDAT_STATE_WRITING;
		wakeup_transmitter = true;
		log_msg = log_msgs[0];
		res = IO_OK;
	} else if (uzdat->state == UZDAT_STATE_WRITING) {
		log_msg = log_msgs[1];
	} else { // IDLE or READ
		log_msg = log_msgs[2];
		wakeup_transmitter = true;
	}
	pthread_mutex_unlock(&uzdat->mutex);

	__LOG_DATA("%s: ", log_msg, data);
	if (wakeup_transmitter) uv_async_send(&uzdat->async_write);

	return res;
}

// -----------------------------------------------------------------------
static int uzdat_disconnect(uzdat_t *uzdat)
{
	LOG(L_UZDAT, "Command: DISCONECT");

	uv_async_send(&uzdat->async_disconnect);

	return IO_OK;
}

// -----------------------------------------------------------------------
int uzdat_cmd(struct cchar_unit_proto_t *unit, int dir, int cmd, uint16_t *r_arg)
{
	uzdat_t *uzdat = (uzdat_t*) unit;

	if (dir == IO_IN) {
		switch (cmd) {
			case UZDAT_CMD_SPU:
				LOG(L_UZDAT, "Command: SPU");
				return IO_OK;
			case UZDAT_CMD_READ:
				return uzdat_read(uzdat, r_arg);
			default:
				LOG(L_UZDAT, "Unknown IN command: %i", cmd);
				return IO_NO;
		}
	} else {
		switch (cmd) {
			case UZDAT_CMD_RESET:
				uzdat_reset(unit);
				return IO_OK;
			case UZDAT_CMD_DISCONNECT:
				return uzdat_disconnect(uzdat);
			case UZDAT_CMD_WRITE:
				return uzdat_write(uzdat, r_arg);
			default:
				LOG(L_UZDAT, "Unknown OU command: %i", cmd);
				return IO_NO;
		}
	}
}

// vim: tabstop=4 shiftwidth=4 autoindent
