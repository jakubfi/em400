//  Copyright (c) 2026 Jakub Filipowicz <jakubf@gmail.com>
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

#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <pthread.h>
#include <uv.h>
#include <time.h>

#include "utils/utils.h"
#include "io/defs.h"
#include "io/cchar/cchar.h"
#include "io/cchar/meclo.h"
#include "io/cchar/meclo_prom.h"
#include "io/dev/rtclock.h"
#include "log.h"

#define MECLO_CMD_PROM_RD	(1<<5)
#define MECLO_CMD_HS		(1<<4)
#define MECLO_CMD_MS		(1<<3)
#define MECLO_CMD_DR		(1<<2)
#define MECLO_CMD_TR		(1<<1)
#define MECLO_CMD_IRQ		(1<<0)

#define MECLO_TIME_BIT		(1<<15)
#define MECLO_TIME_DATA		((uint16_t)(~MECLO_TIME_BIT))

#define MECLO_PROM_SIZE 256

#define MECLO_DISPLAY_UPDATE_DELAY_MS 8
#define MECLO_CLOCK_TICK_MS 500
#define MECLO_SECONDS_SWITCH_DELAY_MS 1250
#define MECLO_DATE_SHOW_TIMEOUT_MS 1250

extern uv_loop_t *ioloop;

typedef struct meclo_s meclo_t;
struct meclo_s {
	cchar_unit_t base;

	bool dr, tr, hs, ms, irq;

	bool cycle_seconds; // cycle between showing time and seconds every 2Hz
	bool seconds_read; // display seconds instead of time
	bool seconds_advance; // Internal clock tick is 2Hz, seconds "tick" every other internal tick
	bool clock_stopped; // setting minutes stops the clock
	bool keep_date_read;

	uint8_t month, day, hour, minute, second;
	uint16_t display;
	uint8_t prom_pos;

	int open_handles;
	uv_async_t async_display_update;
	uv_timer_t timer_display_update;

	uv_timer_t timer_clock_tick;

	uv_async_t async_seconds_switch;
	uv_timer_t timer_seconds_switch;

	uv_async_t async_keep_date_read;
	uv_timer_t timer_keep_date_read;

	pthread_mutex_t mutex;
	bool has_int;

	em400_dev_t *dev;
};

void meclo_shutdown(cchar_unit_t *unit);
void meclo_reset(cchar_unit_t *unit);
int meclo_cmd(cchar_unit_t *unit, int dir, int cmd, uint16_t *r_arg);
int meclo_intspec(cchar_unit_t *unit);
bool meclo_has_interrupt(cchar_unit_t *unit);

// -----------------------------------------------------------------------
static uint8_t to_bcd(uint8_t i)
{
	return ((i/10) << 4) | (i%10);
}

// -----------------------------------------------------------------------
static void meclo_try_free(meclo_t *meclo)
{
	if (meclo->open_handles <= 0) {
		LOG(L_MECLO, "No more open handles, MECLO freeing resources");
		if ((meclo->dev) && (meclo->dev->shutdown)) {
			meclo->dev->shutdown((em400_dev_t *) meclo->dev);
		}
		pthread_mutex_destroy(&meclo->mutex);
		free(meclo);
	}
}

// -----------------------------------------------------------------------
static void on_handle_close(uv_handle_t* handle)
{
	meclo_t *meclo = (meclo_t *) uv_handle_get_data(handle);
	meclo->open_handles--;
	meclo_try_free(meclo);
}

// -----------------------------------------------------------------------
// NOTE: call with mutex locked
static void meclo_clock_advance__locked(meclo_t *meclo, int m_days)
{
	if (meclo->seconds_advance) {
		if (++meclo->second > 59) {
			meclo->second = 0;
			if (++meclo->minute > 59) {
				meclo->minute = 0;
				if (++meclo->hour > 23) {
					meclo->hour = 0;
					if (++meclo->day > m_days) {
						meclo->day = 1;
						if (++meclo->month > 12) {
							meclo->month = 1;
						}
					}
				}
			}
		}
	}

	meclo->seconds_advance = !meclo->seconds_advance;
}

// -----------------------------------------------------------------------
// NOTE: call with mutex locked
static void meclo_clock_set__locked(meclo_t *meclo, int m_days)
{
	if (meclo->hs) {
		if (meclo->dr) {
			if (meclo->tr) {
				if (meclo->day < m_days) meclo->day++;
				else meclo->day = 1;
			} else {
				if (meclo->month < 12) meclo->month++;
				else meclo->month = 1;
			}
		} else {
			if (meclo->hour < 23) meclo->hour++;
			else meclo->hour = 0;
		}
	}
	if (meclo->ms) {
		if (meclo->minute < 59) meclo->minute++;
		else meclo->minute = 0;
	}
}

// -----------------------------------------------------------------------
static void meclo_on_clock_tick_timeout(uv_timer_t *handle)
{
	meclo_t *meclo = (meclo_t*) handle->data;

	static const int month_days[] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

	pthread_mutex_lock(&meclo->mutex);

	if (meclo->cycle_seconds) {
		meclo->seconds_read = !meclo->seconds_read;
	}

	int m_days = month_days[meclo->month-1];
	meclo_clock_set__locked(meclo, m_days);
	if (!meclo->clock_stopped) {
		meclo_clock_advance__locked(meclo, m_days);
	}

	bool report_irq = meclo->has_int = meclo->irq;

	pthread_mutex_unlock(&meclo->mutex);

	if (report_irq) {
		cchar_int_trigger(meclo->base.chan);
	}

	uv_async_send(&meclo->async_display_update);
}

// -----------------------------------------------------------------------
static void meclo_on_display_update_timeout(uv_timer_t *handle)
{
	meclo_t *meclo = (meclo_t*) handle->data;

	pthread_mutex_lock(&meclo->mutex);
	if (meclo->dr || meclo->keep_date_read) {
		meclo->display = (to_bcd(meclo->month) << 8) | (to_bcd(meclo->day));
	} else {
		if (meclo->seconds_read) {
			meclo->display = to_bcd(meclo->second);
		} else {
			meclo->display = MECLO_TIME_BIT | (to_bcd(meclo->hour) << 8) | (to_bcd(meclo->minute));
		}
	}
	pthread_mutex_unlock(&meclo->mutex);
}

// -----------------------------------------------------------------------
static void meclo_on_seconds_switch_timeout(uv_timer_t *handle)
{
	meclo_t *meclo = (meclo_t*) uv_handle_get_data((uv_handle_t*) handle);
	LOG(L_MECLO, "Start showing seconds");

	pthread_mutex_lock(&meclo->mutex);
	meclo->seconds_read = true;
	meclo->cycle_seconds = true;
	pthread_mutex_unlock(&meclo->mutex);
}

// -----------------------------------------------------------------------
static void meclo_on_async_display_update(uv_async_t *handle)
{
	meclo_t *meclo = (meclo_t*) uv_handle_get_data((uv_handle_t*) handle);

	if (!uv_is_active((uv_handle_t*) &meclo->timer_display_update)) {
		uv_timer_start(&meclo->timer_display_update, meclo_on_display_update_timeout, MECLO_DISPLAY_UPDATE_DELAY_MS, 0);
	}
}

// -----------------------------------------------------------------------
static void meclo_on_async_seconds_switch(uv_async_t *handle)
{
	meclo_t *meclo = (meclo_t*) uv_handle_get_data((uv_handle_t*) handle);

	if (!uv_is_active((uv_handle_t*) &meclo->timer_seconds_switch)) {
		uv_timer_start(&meclo->timer_seconds_switch, meclo_on_seconds_switch_timeout, MECLO_SECONDS_SWITCH_DELAY_MS, 0);
	}
}

// -----------------------------------------------------------------------
static void meclo_on_keep_date_read_timeout(uv_timer_t *handle)
{
	meclo_t *meclo = (meclo_t*) uv_handle_get_data((uv_handle_t*) handle);
	LOG(L_MECLO, "Stop showing date");

	pthread_mutex_lock(&meclo->mutex);
	meclo->keep_date_read = false;
	pthread_mutex_unlock(&meclo->mutex);

	uv_async_send(&meclo->async_display_update);
}

// -----------------------------------------------------------------------
static void meclo_on_async_keep_date_read(uv_async_t *handle)
{
	meclo_t *meclo = (meclo_t*) uv_handle_get_data((uv_handle_t*) handle);

	if (!uv_is_active((uv_handle_t*) &meclo->timer_keep_date_read)) {
		uv_timer_start(&meclo->timer_keep_date_read, meclo_on_keep_date_read_timeout, MECLO_DATE_SHOW_TIMEOUT_MS, 0);
	}
}

// -----------------------------------------------------------------------
static void meclo_ioloop_teardown(meclo_t *meclo)
{
	uv_close((uv_handle_t *) &meclo->async_display_update, on_handle_close);
	uv_close((uv_handle_t *) &meclo->timer_display_update, on_handle_close);
	uv_close((uv_handle_t *) &meclo->timer_clock_tick, on_handle_close);
	uv_close((uv_handle_t *) &meclo->timer_seconds_switch, on_handle_close);
	uv_close((uv_handle_t *) &meclo->async_seconds_switch, on_handle_close);
	uv_close((uv_handle_t *) &meclo->timer_keep_date_read, on_handle_close);
	uv_close((uv_handle_t *) &meclo->async_keep_date_read, on_handle_close);
}

// -----------------------------------------------------------------------
static int meclo_ioloop_setup(meclo_t *meclo)
{
	int res;

	res = uv_async_init(ioloop, &meclo->async_display_update, meclo_on_async_display_update);
	if (res) goto fail_async_display_update;
	uv_handle_set_data((uv_handle_t*) &meclo->async_display_update, meclo);
	meclo->open_handles++;

	res = uv_timer_init(ioloop, &meclo->timer_display_update);
	if (res) goto fail_timer_display_update;
	uv_handle_set_data((uv_handle_t*) &meclo->timer_display_update, meclo);
	meclo->open_handles++;

	res = uv_timer_init(ioloop, &meclo->timer_clock_tick);
	if (res) goto fail_timer_clock_tick;
	uv_handle_set_data((uv_handle_t*) &meclo->timer_clock_tick, meclo);
	meclo->open_handles++;

	res = uv_timer_init(ioloop, &meclo->timer_seconds_switch);
	if (res) goto fail_timer_seconds_switch;
	uv_handle_set_data((uv_handle_t*) &meclo->timer_seconds_switch, meclo);
	meclo->open_handles++;

	res = uv_async_init(ioloop, &meclo->async_seconds_switch, meclo_on_async_seconds_switch);
	if (res) goto fail_async_seconds_switch;
	uv_handle_set_data((uv_handle_t*) &meclo->async_seconds_switch, meclo);
	meclo->open_handles++;

	res = uv_timer_init(ioloop, &meclo->timer_keep_date_read);
	if (res) goto fail_timer_keep_date_read;
	uv_handle_set_data((uv_handle_t*) &meclo->timer_keep_date_read, meclo);
	meclo->open_handles++;

	res = uv_async_init(ioloop, &meclo->async_keep_date_read, meclo_on_async_keep_date_read);
	if (res) goto fail_async_keep_date_read;
	uv_handle_set_data((uv_handle_t*) &meclo->async_keep_date_read, meclo);
	meclo->open_handles++;

	return E_OK;

fail_async_keep_date_read:
	uv_close((uv_handle_t *) &meclo->timer_keep_date_read, on_handle_close);
fail_timer_keep_date_read:
	uv_close((uv_handle_t *) &meclo->async_seconds_switch, on_handle_close);
fail_async_seconds_switch:
	uv_close((uv_handle_t *) &meclo->timer_seconds_switch, on_handle_close);
fail_timer_seconds_switch:
	uv_close((uv_handle_t *) &meclo->timer_clock_tick, on_handle_close);
fail_timer_clock_tick:
	uv_close((uv_handle_t *) &meclo->timer_display_update, on_handle_close);
fail_timer_display_update:
	uv_close((uv_handle_t *) &meclo->async_display_update, on_handle_close);
fail_async_display_update:
	return LOGERR("MECLO I/O loop resources initialization error: %s", uv_strerror(res));
}

// -----------------------------------------------------------------------
static void meclo_set_current_time(meclo_t *meclo)
{
	time_t cur_time;
	struct tm t;

	time(&cur_time);
	if (localtime_r(&cur_time, &t)) {
		meclo->month = t.tm_mon+1;
		meclo->day = t.tm_mday;
		meclo->hour = t.tm_hour;
		meclo->minute = t.tm_min;
		meclo->second = t.tm_sec;
	} else {
		meclo->month = meclo->day = 1;
		meclo->hour = meclo->minute = meclo->second = 0;
		LOG(L_MECLO, "Setting initial time failed, clock starting at Jan 1st, 00:00");
	}
}

// -----------------------------------------------------------------------
static int meclo_prom_load(meclo_t *meclo, const char *prom_filename)
{
	FILE *f = fopen(prom_filename, "rb");
	if (!f) {
		return LOGERR("Failed to open MECLO PROM image: %s", prom_filename);
	}
	// reads smaller than MECLO_PROM_SIZE are allowed
	int res = fread(meclo_prom_data, sizeof(uint8_t), MECLO_PROM_SIZE, f);
	fclose(f);
	if (res <= 0) {
		return LOGERR("Failed to read MECLO PROM image: %s", prom_filename);
	}
	LOG(L_MECLO, "Loaded %i bytes into MECLO PROM", res);

	return E_OK;
}

// -----------------------------------------------------------------------
cchar_unit_t * meclo_create(int dev_num, em400_dev_t *dev)
{
	LOG(L_MECLO, "Device %i: creating MECLO real time clock", dev_num);

	meclo_t *meclo = (meclo_t*) calloc(1, sizeof(meclo_t));
	if (!meclo) {
		LOGERR("Device %i: failed to allocate memory for MECLO", dev_num);
		return NULL;
	}

	if (pthread_mutex_init(&meclo->mutex, NULL)) {
		LOGERR("Device %i: failed to initialize MECLO mutex", dev_num);
		free(meclo);
		return NULL;
	}

	rtclock_t *rtclock = (rtclock_t*) dev;
	if (rtclock->prom_filename && *rtclock->prom_filename) {
		LOG(L_MECLO, "Loading custom PROM image (max %i bytes) from %s", MECLO_PROM_SIZE, rtclock->prom_filename);
		if (meclo_prom_load(meclo, rtclock->prom_filename) != E_OK) {
			pthread_mutex_destroy(&meclo->mutex);
			free(meclo);
			return NULL;
		}
	} else {
		LOG(L_MECLO, "Using default MECLO PROM");
	}

	meclo->base.num = dev_num;
	meclo->base.shutdown = meclo_shutdown;
	meclo->base.reset = meclo_reset;
	meclo->base.cmd = meclo_cmd;
	meclo->base.intspec = meclo_intspec;
	meclo->base.has_interrupt = meclo_has_interrupt;
	meclo->dev = dev;

	if (meclo_ioloop_setup(meclo) == E_ERR) {
		meclo_try_free(meclo);
		return NULL;
	}

	meclo_set_current_time(meclo);
	// TODO: synchronize to wall-clock, prevent drift
	uv_timer_start(&meclo->timer_clock_tick, meclo_on_clock_tick_timeout, 0, MECLO_CLOCK_TICK_MS);

	return (cchar_unit_t *) meclo;
}

// -----------------------------------------------------------------------
void meclo_shutdown(cchar_unit_t *unit)
{
	if (!unit) return;

	LOG(L_MECLO, "MECLO shutting down");

	meclo_t *meclo = (meclo_t*) unit;

	meclo_ioloop_teardown(meclo);
}

// -----------------------------------------------------------------------
int meclo_intspec(cchar_unit_t *unit)
{
	LOG(L_MECLO, "Command: INTSPEC");

	meclo_t *meclo = (meclo_t*) unit;

	pthread_mutex_lock(&meclo->mutex);
	meclo->has_int = false;
	pthread_mutex_unlock(&meclo->mutex);

	return 0;
}

// -----------------------------------------------------------------------
bool meclo_has_interrupt(cchar_unit_t *unit)
{
	meclo_t *meclo = (meclo_t*) unit;

	pthread_mutex_lock(&meclo->mutex);
	bool has_int = meclo->has_int;
	pthread_mutex_unlock(&meclo->mutex);

	return has_int;
}

// -----------------------------------------------------------------------
void meclo_reset(cchar_unit_t *unit)
{
	LOG(L_MECLO, "MECLO reset");

	meclo_t *meclo = (meclo_t*) unit;

	// resets device, but the clock keeps running
	pthread_mutex_lock(&meclo->mutex);
	meclo->prom_pos = 0;
	meclo->hs = false;
	meclo->ms = false;
	meclo->dr = false;
	meclo->tr = false;
	meclo->irq = false;
	meclo->seconds_read = false;
	meclo->cycle_seconds = false;
	meclo->clock_stopped = false;
	pthread_mutex_unlock(&meclo->mutex);
}

// -----------------------------------------------------------------------
static uint16_t meclo_prom_read(meclo_t *meclo)
{
	pthread_mutex_lock(&meclo->mutex);
	uint8_t pos = meclo->prom_pos;
	// 8-bit meclo->prom_pos is allowed to overflow
	uint8_t data = meclo_prom_data[meclo->prom_pos++];
	pthread_mutex_unlock(&meclo->mutex);

	LOG(L_MECLO, "PROM read @%i: 0x%02x", pos, data);
	return (uint16_t) data;
}

// -----------------------------------------------------------------------
// NOTE: call with mutex locked
static void meclo_decode_cmd__locked(meclo_t *meclo, int cmd)
{
	meclo->hs = !(cmd & MECLO_CMD_HS);
	meclo->ms = !(cmd & MECLO_CMD_MS);
	meclo->dr = cmd & MECLO_CMD_DR;
	meclo->tr = cmd & MECLO_CMD_TR;
	meclo->irq = cmd & MECLO_CMD_IRQ;
}

// -----------------------------------------------------------------------
static uint16_t meclo_clock_op(meclo_t *meclo, int cmd)
{
	uint16_t clock_status;
	bool trigger_seconds_display = false;
	bool trigger_keep_date_read = false;

	pthread_mutex_lock(&meclo->mutex);

	meclo_decode_cmd__locked(meclo, cmd);

	if (meclo->tr) {
		// tr triggers displaying seconds after a delay,
		// if pressed alone and not already triggered
		if (!meclo->dr && !meclo->cycle_seconds) {
			trigger_seconds_display = true;
		}
		// tr also starts the clock
		meclo->clock_stopped = false;
	} else {
		// no tr => stop showing seconds
		meclo->seconds_read = false;
		meclo->cycle_seconds = false;
	}

	if (meclo->ms) {
		// setting minutes stops the clock and resets seconds
		meclo->clock_stopped = true;
		meclo->second = 0;
	}

	// keep displaying date for 1.25s when dr alone is pressed alone
	if (meclo->dr && !meclo->tr) {
		trigger_keep_date_read = true;
		meclo->keep_date_read = true;
	}

	clock_status = meclo->display;

	pthread_mutex_unlock(&meclo->mutex);

	// NOTE: hs/ms/dr/tr/irq read without lock - intentional, unlikely, don't care
	LOG(L_MECLO, "DISPLAY: %s: %04x / OPERATION: %s%s%s%s%s",
		clock_status & MECLO_TIME_BIT ? "time" : "date",
		clock_status & MECLO_TIME_DATA,
		meclo->hs ? "hs " : "",
		meclo->ms ? "ms " : "",
		meclo->dr ? "dr " : "",
		meclo->tr ? "tr " : "",
		meclo->irq ? "irq " : ""
		);

	uv_async_send(&meclo->async_display_update);
	if (trigger_keep_date_read) {
		uv_async_send(&meclo->async_keep_date_read);
	}
	if (trigger_seconds_display) {
		uv_async_send(&meclo->async_seconds_switch);
	}

	return clock_status;
}

// -----------------------------------------------------------------------
int meclo_cmd(cchar_unit_t *unit, int dir, int cmd, uint16_t *r_arg)
{
	meclo_t *meclo = (meclo_t*) unit;

	if (dir != IO_IN) return IO_NO;

	if ((cmd & MECLO_CMD_PROM_RD)) {
		*r_arg = meclo_prom_read(meclo);
	} else {
		*r_arg = meclo_clock_op(meclo, cmd);
	}

	return IO_OK;
}

// vim: tabstop=4 shiftwidth=4 autoindent
