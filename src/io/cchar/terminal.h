//  Copyright (c) 2024 Jakub Filipowicz <jakubf@gmail.com>
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

#ifndef __TERMINAL_ACTUAL_H__
#define __TERMINAL_ACTUAL_H__

#include <pthread.h>
#include <uv.h>

#define TERMINAL_BUF_SIZE 1024

typedef struct termial_s terminal_t;

typedef void (*on_data_received_cb)(void *ptr, char data);
typedef void (*on_data_sent_cb)(void *ptr);
typedef void (*reset_fn)(terminal_t *terminal);
typedef void (*destroy_fn)(terminal_t *terminal);
typedef int (*write_fn)(terminal_t *terminal, char data);

struct termial_s {
	int port;
	int speed;

	pthread_mutex_t buf_mutex;
	char rdbuf[TERMINAL_BUF_SIZE];
	char *rdbuf_r;
	char *rdbuf_w;
	char *rdbuf_end;
	int rdbuf_count;
	bool sender_busy;

	uv_tcp_t *client;
	uv_timer_t timer_write;
	uv_timer_t timer_read;
	uv_tcp_t tcp_handle;

	on_data_received_cb on_data_received;
	on_data_sent_cb on_data_sent;
	void *controller;

	reset_fn reset;
	destroy_fn destroy;
	write_fn write;
};

terminal_t * terminal_create(void *controller, on_data_received_cb cbr, on_data_sent_cb cbs, unsigned port, unsigned speed);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
