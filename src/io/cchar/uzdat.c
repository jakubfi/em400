//  Copyright (c) 2013-2025 Jakub Filipowicz <jakubf@gmail.com>
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
#include <uv.h>

#include "io/defs.h"
#include "io/cchar/cchar.h"
#include "io/cchar/uzdat.h"
#include "io/dev/terminal.h"

#include "log.h"

#define XFER_DIR_SWITCH_DELAY 2 // miliseconds

extern uv_loop_t *ioloop;

typedef struct uzdat_s uzdat_t;
struct uzdat_s {
	cchar_unit_t base;

	pthread_mutex_t mutex;
	int intspec;
	int state;
	int dir;
	bool xfer_busy;
	char buf_wr;
	int buf_rd;

	em400_dev_t *dev;

	uv_async_t async_write;
	uv_async_t async_switch_transmit;
	uv_timer_t timer_switch_transmit;
	int open_handles;
};

enum uzdat_states {
	UZDAT_STATE_OFF,
	UZDAT_STATE_OK,
	UZDAT_STATE_EN
};

enum uzdat_dirs {
	UZDAT_DIR_NONE,
	UZDAT_DIR_IN,
	UZDAT_DIR_OUT
};

enum uzdat_ou_commands {
	UZDAT_CMD_RESET			= 0b100000, // reset
	UZDAT_CMD_DISCONNECT	= 0b101000, // disconnect device (soft reset)
	UZDAT_CMD_WRITE			= 0b110000, // write
};

enum uzdat_in_commands {
	UZDAT_CMD_SPU			= 0b100000, // check device presence
	UZDAT_CMD_READ			= 0b101000, // read
};

enum cchar_uzdat_interrupts {
	UZDAT_INT_OUTDATED	= 0, // interrupt out of date
	UZDAT_INT_READY		= 1, // ready again
	UZDAT_INT_TOO_SLOW	= 5, // transmission too slow
};

void uzdat_shutdown(cchar_unit_t *unit);
void uzdat_reset(cchar_unit_t *unit);
int uzdat_cmd(cchar_unit_t *unit, int dir, int cmd, uint16_t *r_arg);
int uzdat_intspec(cchar_unit_t *unit);
bool uzdat_has_interrupt(cchar_unit_t *unit);

static void uzdat_on_async_write(uv_async_t *handle);
static void uzdat_on_async_switch_dir(uv_async_t *handle);

void uzdat_on_data_received(uzdat_t *uzdat, char data);
void uzdat_on_data_sent(uzdat_t *uzdat);

// -----------------------------------------------------------------------
static void uzdat_try_free(uzdat_t *uzdat)
{
	if (uzdat->open_handles <= 0) {
		LOG(L_UZDAT, "No more open handles, UZDAT freeing resources");
		pthread_mutex_destroy(&uzdat->mutex);
		free(uzdat);
	}
}

// -----------------------------------------------------------------------
static void on_handle_close(uv_handle_t* handle)
{
	uzdat_t *uzdat = (uzdat_t *) uv_handle_get_data(handle);
	uzdat->open_handles--;
	uzdat_try_free(uzdat);
}

// -----------------------------------------------------------------------
static void uzdat_ioloop_teardown(uzdat_t *uzdat)
{
	if (!uv_is_closing((uv_handle_t *) &uzdat->async_write)) {
		uv_close((uv_handle_t *) &uzdat->async_write, on_handle_close);
	}
	if (!uv_is_closing((uv_handle_t *) &uzdat->async_switch_transmit)) {
		uv_close((uv_handle_t *) &uzdat->async_switch_transmit, on_handle_close);
	}
	if (!uv_is_closing((uv_handle_t *) &uzdat->timer_switch_transmit)) {
		uv_close((uv_handle_t *) &uzdat->timer_switch_transmit, on_handle_close);
	}
}

// -----------------------------------------------------------------------
static int uzdat_ioloop_setup(uzdat_t *uzdat)
{
	int res;

	res = uv_async_init(ioloop, &uzdat->async_write, uzdat_on_async_write);
	if (res) {
		return LOGERR("UZDAT async_write handler init error: %s", uv_strerror(res));
	}
	uv_handle_set_data((uv_handle_t*) &uzdat->async_write, uzdat);
	uzdat->open_handles++;

	res = uv_async_init(ioloop, &uzdat->async_switch_transmit, uzdat_on_async_switch_dir);
	if (res) {
		uv_close((uv_handle_t *) &uzdat->async_write, on_handle_close);
		return LOGERR("UZDAT async_switch_transmit handler init error: %s", uv_strerror(res));
	}
	uv_handle_set_data((uv_handle_t*) &uzdat->async_switch_transmit, uzdat);
	uzdat->open_handles++;

	res = uv_timer_init(ioloop, &uzdat->timer_switch_transmit);
	if (res) {
		uv_close((uv_handle_t *) &uzdat->async_write, on_handle_close);
		uv_close((uv_handle_t *) &uzdat->async_switch_transmit, on_handle_close);
		return LOGERR("UZDAT timer init error: %s", uv_strerror(res));
	}
	uv_handle_set_data((uv_handle_t*) &uzdat->timer_switch_transmit, uzdat);
	uzdat->open_handles++;

	return E_OK;
}

// -----------------------------------------------------------------------
cchar_unit_t * uzdat_create(int dev_num, em400_dev_t *dev)
{
	LOG(L_UZDAT, "Device %i: creating UZDAT terminal controller", dev_num);

	uzdat_t *uzdat = (uzdat_t*) calloc(1, sizeof(uzdat_t));
	if (!uzdat) {
		LOGERR("Device %i: failed to allocate memory for UZDAT", dev_num);
		return NULL;
	}

	if (pthread_mutex_init(&uzdat->mutex, NULL)) {
		LOGERR("Device %i: failed to initialize UZDAT mutex", dev_num);
		free(uzdat);
		return NULL;
	}

	uzdat->base.num = dev_num;
	uzdat->base.shutdown = uzdat_shutdown;
	uzdat->base.reset = uzdat_reset;
	uzdat->base.cmd = uzdat_cmd;
	uzdat->base.intspec = uzdat_intspec;
	uzdat->base.has_interrupt = uzdat_has_interrupt;

	uzdat_reset((cchar_unit_t *) uzdat);

	uzdat->dev = dev;
	// TODO: generic device callback registration?
	terminal_register_callbacks((terminal_t *) uzdat->dev, uzdat, (void*) uzdat_on_data_received, (void*) uzdat_on_data_sent);

	if (uzdat_ioloop_setup(uzdat) == E_ERR) {
		uzdat_try_free(uzdat);
		return NULL;
	}

	return (cchar_unit_t *) uzdat;
}

// -----------------------------------------------------------------------
void uzdat_on_data_received(uzdat_t *uzdat, char data)
{
	LOGCHAR(L_UZDAT, "%s: ", "Terminal sent data", data);

	int trigger_interrupt = false;

	pthread_mutex_lock(&uzdat->mutex);
	// Receive data when in "read" mode (i.e. also ignore incomming data after reset)
	const bool reading = (uzdat->state != UZDAT_STATE_OFF) && (uzdat->dir == UZDAT_DIR_IN);
	// Also receive backspace (or del) interpreted as "operator request" in "off" state.
	// Terminals in CROOK seem to work that way, and it seems to be a modification done
	// to UZ-DAT board, since vanilla UZ-DAT doesn't support operator request.
	// TODO: analyze actual UZDAT modification (it sends real OPRQ) and modify the behavior
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
		cchar_int_trigger(uzdat->base.chan);
	}
}

// -----------------------------------------------------------------------
void uzdat_on_data_sent(uzdat_t *uzdat)
{
	LOG(L_UZDAT, "Data sent to terminal");

	int trigger_interrupt = false;

	pthread_mutex_lock(&uzdat->mutex);
	if (uzdat->state != UZDAT_STATE_OFF) { // if UZDAT has not been reset
		uzdat->intspec = UZDAT_INT_READY;
		uzdat->xfer_busy = false;
		trigger_interrupt = true;
	}
	pthread_mutex_unlock(&uzdat->mutex);

	if (trigger_interrupt) {
		cchar_int_trigger(uzdat->base.chan);
	}
}

// -----------------------------------------------------------------------
static void uzdat_on_async_write(uv_async_t *handle)
{
	uzdat_t *uzdat = (uzdat_t*) handle->data;

	pthread_mutex_lock(&uzdat->mutex);
	char data = uzdat->buf_wr;
	pthread_mutex_unlock(&uzdat->mutex);

	if (!uzdat->dev) {
		LOGCHAR(L_UZDAT, "%s: ", "Failed write, no terminal connected", data);
		return;
	}

	int res = uzdat->dev->write(uzdat->dev, data);
	if (res == 0) {
		LOGCHAR(L_UZDAT, "%s: ", "Written to terminal", data);
	} else {
		LOGCHAR(L_UZDAT, "%s: ", "Failed write to terminal", data);
	}
}

// -----------------------------------------------------------------------
void uzdat_shutdown(cchar_unit_t *unit)
{
	if (!unit) return;

	LOG(L_UZDAT, "UZDAT shutting down");

	uzdat_t *uzdat = (uzdat_t*) unit;

	uzdat_ioloop_teardown(uzdat);
	if ((uzdat->dev) && (uzdat->dev->shutdown)) {
		uzdat->dev->shutdown((em400_dev_t *) uzdat->dev);
	}
}

// -----------------------------------------------------------------------
int uzdat_intspec(cchar_unit_t *unit)
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
bool uzdat_has_interrupt(cchar_unit_t *unit)
{
	uzdat_t *uzdat = (uzdat_t*) unit;

	pthread_mutex_lock(&uzdat->mutex);
	int intspec = uzdat->intspec;
	pthread_mutex_unlock(&uzdat->mutex);

	return intspec ? true : false;
}

// -----------------------------------------------------------------------
static int uzdat_read(uzdat_t *uzdat, uint16_t *r_arg)
{
	int ret;
	LOG(L_UZDAT, "Command: READ");

	pthread_mutex_lock(&uzdat->mutex);
	uzdat->dir = UZDAT_DIR_IN;
	int data = uzdat->buf_rd & 0xff;
	if (uzdat->buf_rd >= 0) {
		*r_arg = (*r_arg & 0xff00) | (data &0xff);
		uzdat->buf_rd = -1;
		uzdat->state = UZDAT_STATE_OK;
		ret = IO_OK;
	} else {
		uzdat->state = UZDAT_STATE_EN;
		ret = IO_EN;
	}
	pthread_mutex_unlock(&uzdat->mutex);

	if (ret == IO_OK) {
		LOGCHAR(L_UZDAT, "%s: ", "READ ready, received: ", data & 0xff);
	} else {
		LOG(L_UZDAT, "Buffer empty, nothing to read");
	}

	return ret;
}

// -----------------------------------------------------------------------
static void uzdat_on_transmit_switch_timeout(uv_timer_t *handle)
{
	uzdat_t *uzdat = (uzdat_t*) handle->data;

	bool trigger_interrupt = false;

	LOG(L_UZDAT, "Switched direction to transmit");

	pthread_mutex_lock(&uzdat->mutex);
	// only if UZDAT has not been reset
	if (uzdat->state != UZDAT_STATE_OFF) {
		uzdat->dir = UZDAT_DIR_OUT;
		uzdat->intspec = UZDAT_INT_READY;
		trigger_interrupt = true;
	}
	pthread_mutex_unlock(&uzdat->mutex);

	if (trigger_interrupt) {
		cchar_int_trigger(uzdat->base.chan);
	}
}

// -----------------------------------------------------------------------
static void uzdat_on_async_switch_dir(uv_async_t *handle)
{
	uzdat_t *uzdat = (uzdat_t*) handle->data;

	uv_timer_start(&uzdat->timer_switch_transmit, uzdat_on_transmit_switch_timeout, XFER_DIR_SWITCH_DELAY, 0);
}

// -----------------------------------------------------------------------
static int uzdat_write(uzdat_t *uzdat, const uint16_t *r_arg)
{
	static const char *log_msgs[] = {
		"transceiver ready, sending",
		"transceiver not ready, not sending",
	};
	const char *log_msg = NULL;
	uv_async_t *trigger = NULL;
	int ret;

	char data = *r_arg & 0xff;

	LOG(L_UZDAT, "Command: WRITE");

	pthread_mutex_lock(&uzdat->mutex);
	if ((!uzdat->xfer_busy) && (uzdat->dir == UZDAT_DIR_OUT)) {
		uzdat->state = UZDAT_STATE_OK;
		uzdat->buf_wr = data;
		uzdat->xfer_busy = true;
		trigger = &uzdat->async_write;
		log_msg = log_msgs[0];
		ret = IO_OK;
	} else {
		if (uzdat->dir != UZDAT_DIR_OUT) {
			trigger = &uzdat->async_switch_transmit;
		}
		uzdat->state = UZDAT_STATE_EN;
		log_msg = log_msgs[1];
		ret = IO_EN;
	}
	pthread_mutex_unlock(&uzdat->mutex);

	LOGCHAR(L_UZDAT, "%s: ", log_msg, data);
	if (trigger) {
		if (uv_async_send(trigger)) {
			LOG(L_UZDAT, "uzdat_write async trigger failed");
		}
	}
	return ret;
}

// -----------------------------------------------------------------------
static int uzdat_disconnect(uzdat_t *uzdat)
{
	LOG(L_UZDAT, "Command: DISCONNECT");

	int ret;

	pthread_mutex_lock(&uzdat->mutex);
	if (!uzdat->xfer_busy) {
		uzdat->state = UZDAT_STATE_OFF;
		uzdat->dir = UZDAT_DIR_NONE;
		uzdat->xfer_busy = false;
		uzdat->buf_rd = -1;
		ret = IO_OK;
	} else {
		uzdat->state = UZDAT_STATE_EN;
		ret = IO_EN;
	}
	pthread_mutex_unlock(&uzdat->mutex);

	return ret;
}

// -----------------------------------------------------------------------
void uzdat_reset(cchar_unit_t *unit)
{
	LOG(L_UZDAT, "UZDAT reset");

	uzdat_t *uzdat = (uzdat_t*) unit;

	pthread_mutex_lock(&uzdat->mutex);
	uzdat->state = UZDAT_STATE_OFF;
	uzdat->dir = UZDAT_DIR_NONE;
	uzdat->xfer_busy = false;
	uzdat->buf_rd = -1;
	pthread_mutex_unlock(&uzdat->mutex);
}

// -----------------------------------------------------------------------
int uzdat_cmd(cchar_unit_t *unit, int dir, int cmd, uint16_t *r_arg)
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
