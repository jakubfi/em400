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

#include <stdatomic.h>
#include <uv.h>

#include "io/defs.h"
#include "io/cchar/uzdat.h"
#include "io/cchar/terminal.h"

#include "log.h"
#include "cfg.h"

extern uv_loop_t *ioloop;

static void uzdat_on_async_write(uv_async_t *handle);
static void uzdat_on_async_switch_dir(uv_async_t *handle);
void uzdat_on_data_received(uzdat_t *uzdat, char data);
void uzdat_on_data_sent(uzdat_t *uzdat);


// -----------------------------------------------------------------------
static void uzdat_ioloop_setup(uzdat_t *uzdat)
{
	uv_async_init(ioloop, &uzdat->async_write, uzdat_on_async_write);
	uv_handle_set_data((uv_handle_t*) &uzdat->async_write, uzdat);

	uv_async_init(ioloop, &uzdat->async_switch_transmit, uzdat_on_async_switch_dir);
	uv_handle_set_data((uv_handle_t*) &uzdat->async_switch_transmit, uzdat);

	uv_timer_init(ioloop, &uzdat->timer_switch_transmit);
	uv_handle_set_data((uv_handle_t*) &uzdat->timer_switch_transmit, uzdat);
}

// -----------------------------------------------------------------------
static void uzdat_ioloop_teardown(uzdat_t *uzdat)
{
	uv_close((uv_handle_t *) &uzdat->async_write, NULL);
	uv_close((uv_handle_t *) &uzdat->async_switch_transmit, NULL);
	uv_close((uv_handle_t *) &uzdat->timer_switch_transmit, NULL);
}

// -----------------------------------------------------------------------
cchar_unit_proto_t * uzdat_create(em400_cfg *cfg, int ch_num, int dev_num)
{
	uzdat_t *uzdat = (uzdat_t*) calloc(1, sizeof(uzdat_t));
	if (!uzdat) {
		LOGERR("Failed to allocate memory for UZDAT: %i.%i", ch_num, dev_num);
		goto fail;
	}

	LOG(L_UZDAT, "Creating UZDAT terminal controller");

	const char *transport = cfg_fgetstr(cfg, "dev%i.%i:transport", ch_num, dev_num);
	unsigned speed = cfg_fgetint(cfg, "dev%i.%i:speed", ch_num, dev_num);
	unsigned port = cfg_fgetint(cfg, "dev%i.%i:port", ch_num, dev_num);

	if (transport && strcasecmp(transport, "tcp")) {
		LOGERR("Transport '%s' not supported. TCP only.", transport);
		goto fail;
	}
	if (!port || !speed) {
		LOGERR("TCP terminal needs port and emulated speed to be set.");
		goto fail;
	}

	uzdat_reset((cchar_unit_proto_t *) uzdat);

	uzdat->terminal = terminal_create(uzdat, (void*) uzdat_on_data_received, (void*) uzdat_on_data_sent, port, speed);
	if (!uzdat->terminal) {
		LOGERR("Failed to create terminal");
		goto fail;
	}

	uzdat_ioloop_setup(uzdat);

	return (cchar_unit_proto_t *) uzdat;

fail:
	uzdat_free((cchar_unit_proto_t *) uzdat);
	return NULL;
}

// -----------------------------------------------------------------------
void uzdat_on_data_received(uzdat_t *uzdat, char data)
{
	LOGCHAR(L_UZDAT, "%s: ", "Data received", data)

	int trigger_interrupt = false;

	pthread_mutex_lock(&uzdat->mutex);
	// Receive data when in "read" mode (i.e. also ignore incomming data after reset)
	const bool reading = (uzdat->state != UZDAT_STATE_OFF) && (uzdat->dir == UZDAT_DIR_IN);
	// Also receive backspace (or del) interpreted as "operator request" in "off" state.
	// Terminals in CROOK seem to work that way, and it seems to be a modification done
	// to UZ-DAT board, since vanilla UZ-DAT doesn't support operator request.
	const bool oprq = (uzdat->state == UZDAT_STATE_OFF) && ((data == 8) || (data == 127));
	if (reading || oprq) {
		if (uzdat->buf_rd >= 0) {
			uzdat->intspec = UZDAT_INT_TOO_SLOW;
		} else {
			uzdat->intspec = UZDAT_INT_READY;
		}
		uzdat->buf_rd = data;
		trigger_interrupt = true;
	}
	pthread_mutex_unlock(&uzdat->mutex);

	if (trigger_interrupt) {
		cchar_int_trigger(uzdat->proto.chan);
	}
}

// -----------------------------------------------------------------------
void uzdat_on_data_sent(uzdat_t *uzdat)
{
	LOG(L_UZDAT, "Data sent");

	int trigger_interrupt = false;

	pthread_mutex_lock(&uzdat->mutex);
	if (uzdat->state != UZDAT_STATE_OFF) { // if UZDAT has not been reset
		uzdat->intspec = UZDAT_INT_READY;
		uzdat->xfer_busy = false;
		trigger_interrupt = true;
	}
	pthread_mutex_unlock(&uzdat->mutex);

	if (trigger_interrupt) {
		cchar_int_trigger(uzdat->proto.chan);
	}
}

// -----------------------------------------------------------------------
static void uzdat_on_async_write(uv_async_t *handle)
{
	uzdat_t *uzdat = (uzdat_t*) handle->data;

	pthread_mutex_lock(&uzdat->mutex);
	char data = uzdat->buf_wr;
	int dir = uzdat->dir;
	pthread_mutex_unlock(&uzdat->mutex);

	if (dir == UZDAT_DIR_OUT) {
		int res = uzdat->terminal->write(uzdat->terminal, data);
		if (res == 0) {
			LOGCHAR(L_UZDAT, "%s: ", "Written to terminal", data)
		} else {
			LOGCHAR(L_UZDAT, "%s: ", "Failed write to terminal", data)
		}
	} else {
		LOGCHAR(L_UZDAT, "%s: ", "Not in sending mode, ignored", data)
	}
}

// -----------------------------------------------------------------------
void uzdat_shutdown(cchar_unit_proto_t *unit)
{
	if (!unit) return;

	LOG(L_UZDAT, "UZDAT shutting down");

	uzdat_t *uzdat = (uzdat_t*) unit;

	uzdat_ioloop_teardown(uzdat);
	uzdat->terminal->destroy(uzdat->terminal);

}

// -----------------------------------------------------------------------
void uzdat_free(cchar_unit_proto_t *unit)
{
	if (!unit) return;

	LOG(L_UZDAT, "UZDAT freeing resources");

	uzdat_t *uzdat = (uzdat_t*) unit;
	if (uzdat->terminal) uzdat->terminal->free(uzdat->terminal);
	free(uzdat);
}

// -----------------------------------------------------------------------
int uzdat_intspec(cchar_unit_proto_t *unit)
{
	LOG(L_UZDAT, "Command: INTSPEC");
	uzdat_t *uzdat = (uzdat_t*) unit;

	pthread_mutex_lock(&uzdat->mutex);
	int spec = uzdat->intspec;
	uzdat->intspec = UZDAT_INT_OUTDATED;
	uzdat->state = UZDAT_STATE_OK;
	pthread_mutex_unlock(&uzdat->mutex);

	return spec;
}

// -----------------------------------------------------------------------
bool uzdat_has_interrupt(cchar_unit_proto_t *unit)
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
	uzdat->dir = UZDAT_DIR_IN;
	int data = uzdat->buf_rd & 0xff;
	if (uzdat->buf_rd >= 0) {
		*r_arg = (*r_arg & 0xff00) | (data &0xff);
		uzdat->buf_rd = -1;
		uzdat->state = UZDAT_STATE_OK;
		res = IO_OK;
	} else {
		uzdat->state = UZDAT_STATE_EN;
		res = IO_EN;
	}
	pthread_mutex_unlock(&uzdat->mutex);

	if (res == IO_OK) {
		LOGCHAR(L_UZDAT, "%s: ", "READ ready, received: ", data & 0xff)
	} else {
		LOG(L_UZDAT, "Buffer empty, nothing to read");
	}

	return res;
}

// -----------------------------------------------------------------------
static void uzdat_on_transmit_switch_timeout(uv_timer_t *handle)
{
	uzdat_t *uzdat = (uzdat_t*) handle->data;

	bool trigger_interrupt = false;

	LOG(L_UZDAT, "Switched direction to transmit");

	pthread_mutex_lock(&uzdat->mutex);
	if (uzdat->state != UZDAT_STATE_OFF) { // if UZDAT has not been reset
		uzdat->dir = UZDAT_DIR_OUT;
		uzdat->intspec = UZDAT_INT_READY;
		trigger_interrupt = true;
	}
	pthread_mutex_unlock(&uzdat->mutex);

	if (trigger_interrupt) {
		cchar_int_trigger(uzdat->proto.chan);
	}
}

// -----------------------------------------------------------------------
static void uzdat_on_async_switch_dir(uv_async_t *handle)
{
	uzdat_t *uzdat = (uzdat_t*) handle->data;

	uv_timer_start(&uzdat->timer_switch_transmit, uzdat_on_transmit_switch_timeout, 2, 0);
}

// -----------------------------------------------------------------------
static int uzdat_write(uzdat_t *uzdat, uint16_t *r_arg)
{
	static const char *log_msgs[] = {
		"transceiver ready, sending",
		"transceiver not ready, not sending",
	};
	const char *log_msg = NULL;
	uv_async_t *trigger = NULL;
	int res;

	char data = *r_arg & 0xff;

	LOG(L_UZDAT, "Command: WRITE");

	pthread_mutex_lock(&uzdat->mutex);
	if ((!uzdat->xfer_busy) && (uzdat->dir == UZDAT_DIR_OUT)) {
		uzdat->state = UZDAT_STATE_OK;
		uzdat->buf_wr = data;
		uzdat->xfer_busy = true;
		trigger = &uzdat->async_write;
		log_msg = log_msgs[0];
		res = IO_OK;
	} else {
		if (uzdat->dir != UZDAT_DIR_OUT) {
			trigger = &uzdat->async_switch_transmit;
		}
		uzdat->state = UZDAT_STATE_EN;
		log_msg = log_msgs[1];
		res = IO_EN;
	}
	pthread_mutex_unlock(&uzdat->mutex);

	LOGCHAR(L_UZDAT, "%s: ", log_msg, data);
	if (trigger) uv_async_send(trigger);

	return res;
}

// -----------------------------------------------------------------------
static int uzdat_disconnect(uzdat_t *uzdat)
{
	LOG(L_UZDAT, "Command: DISCONECT");

	int ret;

	pthread_mutex_lock(&uzdat->mutex);
	if (!uzdat->xfer_busy) {
		uzdat->state = UZDAT_STATE_OFF;
		uzdat->dir = UZDAT_DIR_NONE;
		uzdat->xfer_busy = false;
		uzdat->buf_rd = -1;
		pthread_mutex_unlock(&uzdat->mutex);
		ret = IO_OK;
	} else {
		uzdat->state = UZDAT_STATE_EN;
		ret = IO_EN;
	}
	pthread_mutex_unlock(&uzdat->mutex);

	return ret;
}

// -----------------------------------------------------------------------
void uzdat_reset(cchar_unit_proto_t *unit)
{
	uzdat_t *uzdat = (uzdat_t*) unit;

	pthread_mutex_lock(&uzdat->mutex);
	uzdat->state = UZDAT_STATE_OFF;
	uzdat->dir = UZDAT_DIR_NONE;
	uzdat->xfer_busy = false;
	uzdat->buf_rd = -1;
	pthread_mutex_unlock(&uzdat->mutex);
}

// -----------------------------------------------------------------------
int uzdat_cmd(cchar_unit_proto_t *unit, int dir, int cmd, uint16_t *r_arg)
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
				LOG(L_UZDAT, "Command: RESET");
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
