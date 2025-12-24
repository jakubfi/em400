//  Copyright (c) 2024-2025 Jakub Filipowicz <jakubf@gmail.com>
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

#ifndef TERMINAL_H
#define TERMINAL_H

#include <pthread.h>
#include <uv.h>
#include <stdbool.h>

#include "io/dev2/dev2.h"

#define TERMINAL_BUF_SIZE 1024

typedef struct terminal terminal_t;

typedef void (*on_data_received_cb)(void *ptr, char data);
typedef void (*on_data_sent_cb)(void *ptr);

struct terminal {
	struct em400_dev2 base;

	int port;
	int delay_ms;

	pthread_mutex_t mutex; // required due to reset comming from another thread
	char rdbuf[TERMINAL_BUF_SIZE];
	char *rdbuf_r;
	char *rdbuf_w;
	char *rdbuf_end;
	char wrbuf;
	int rdbuf_count;
	bool sender_busy;

	uv_tcp_t *client;
	uv_timer_t timer_write;
	uv_timer_t timer_read;
	uv_tcp_t tcp_handle;
	int open_handles;

	// TODO: generic device callback registration?
	on_data_received_cb on_data_received;
	on_data_sent_cb on_data_sent;
	void *controller;
};

em400_dev_t * terminal_create(unsigned port, unsigned speed);
void terminal_register_callbacks(terminal_t * terminal, void *controller, on_data_received_cb recv_cb, on_data_sent_cb sent_cb);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
