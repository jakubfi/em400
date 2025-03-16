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

#include <uv.h>
#include <pthread.h>

#include "log.h"
#include "terminal.h"

extern uv_loop_t *ioloop;

int terminal_write(terminal_t *terminal, char c);

// -----------------------------------------------------------------------
static void term_buf_reset(terminal_t *terminal)
{
	pthread_mutex_lock(&terminal->buf_mutex);
	terminal->rdbuf_end = terminal->rdbuf + TERMINAL_BUF_SIZE-1;
	terminal->rdbuf_r = terminal->rdbuf;
	terminal->rdbuf_w = terminal->rdbuf;
	terminal->rdbuf_count = 0;
	terminal->sender_busy = false;
	pthread_mutex_unlock(&terminal->buf_mutex);
}

// -----------------------------------------------------------------------
static int term_buf_get(terminal_t *terminal)
{
	int data;

	pthread_mutex_lock(&terminal->buf_mutex);
	if (terminal->rdbuf_count <= 0) {
		data = -1;
	} else {
		data = (unsigned char) *terminal->rdbuf_r;
		terminal->rdbuf_r++;
		terminal->rdbuf_count--;
		if (terminal->rdbuf_r > terminal->rdbuf_end) {
			terminal->rdbuf_r = terminal->rdbuf;
		}
	}
	pthread_mutex_unlock(&terminal->buf_mutex);

	return data;
}

// -----------------------------------------------------------------------
static int term_buf_append(terminal_t *terminal, char *c, int len)
{
	int ret = 0;

	pthread_mutex_lock(&terminal->buf_mutex);
	while (len > 0) {
		if (terminal->rdbuf_count >= TERMINAL_BUF_SIZE) {
			ret = -1;
			pthread_mutex_unlock(&terminal->buf_mutex);
			break;
		} else {
			*terminal->rdbuf_w = *c;
			terminal->rdbuf_w++;
			terminal->rdbuf_count++;
			if (terminal->rdbuf_w > terminal->rdbuf_end) {
				terminal->rdbuf_w = terminal->rdbuf;
			}
		}
		c++;
		len--;
	}
	pthread_mutex_unlock(&terminal->buf_mutex);

	return ret;
}

// -----------------------------------------------------------------------
static void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
	buf->base = (char*) malloc(suggested_size);
	buf->len = suggested_size;
}

// -----------------------------------------------------------------------
static void on_tcp_close(uv_handle_t* handle)
{
	terminal_t *terminal = (terminal_t *) handle->data;

	LOG(L_TERM, "Terminal TCP connection closed");
	free(handle);
	terminal->client = NULL;
}

// -----------------------------------------------------------------------
static void on_read_delay_timeout(uv_timer_t *handle)
{
	terminal_t *terminal = (terminal_t*) handle->data;

	if (terminal->rdbuf_count > 0) {
		terminal->on_data_received(terminal->controller, term_buf_get(terminal));
		uv_timer_start(&terminal->timer_write, on_read_delay_timeout, 1, 0);
	}
}

// -----------------------------------------------------------------------
static void on_tcp_read(uv_stream_t *handle, ssize_t nread, const uv_buf_t *buf)
{
	int res = 0;
	terminal_t *terminal = (terminal_t *) handle->data;

	if (nread < 0) {
		if (nread != UV_EOF) {
			LOG(L_TERM, "Terminal TCP Read error: %s", uv_err_name(nread));
		} else {
			uv_close((uv_handle_t*) handle, on_tcp_close);
		}
	} else {
		LOG(L_TERM, "Terminal read TCP data (%d bytes)", nread);
		res = term_buf_append(terminal, buf->base, nread);
	}

	free(buf->base);

	if (res >= 0) {
		uv_timer_start(&terminal->timer_write, on_read_delay_timeout, 1, 0);
	}
}

// -----------------------------------------------------------------------
static void on_tcp_reject_user(uv_write_t *req, int status)
{
	uv_close((uv_handle_t*) req->handle, NULL);
	free(req->bufs);
	free(req);
}

// -----------------------------------------------------------------------
static void on_new_tcp_connection(uv_stream_t *handle, int status)
{
	if (status < 0) {
		LOG(L_TERM, "Terminal new TCP connection error: %s", uv_strerror(status));
		return;
	}
	uv_tcp_t *client = (uv_tcp_t *) malloc(sizeof(uv_tcp_t));

	uv_tcp_init(ioloop, client);
	uv_handle_set_data((uv_handle_t*) client, handle->data);

	if (uv_accept(handle, (uv_stream_t*) client)) {
		LOG(L_TERM, "Terminal failed to accept new TCP connection");
		uv_close((uv_handle_t*) client, on_tcp_close);
		return;
	}

	terminal_t *terminal = (terminal_t *) handle->data;

	if (terminal->client) {
		LOG(L_TERM, "Terminal already connected, rejecting new connection");
		uv_write_t *req = (uv_write_t*) malloc(sizeof(uv_write_t));
		static char message[] = "Terminal already connected. Bye.\n";
		uv_buf_t buf = uv_buf_init(message, strlen(message));
		uv_write((uv_write_t*) req, (uv_stream_t *) client, &buf, 1, on_tcp_reject_user);
		return;
	}

	LOG(L_TERM, "Terminal accepted new TCP connection");
	terminal->client = client;
	uv_read_start((uv_stream_t*) client, alloc_buffer, on_tcp_read);
}

// -----------------------------------------------------------------------
static void on_write_delay_timeout(uv_timer_t *handle)
{
	terminal_t *terminal = (terminal_t*) handle->data;

	pthread_mutex_lock(&terminal->buf_mutex);
	terminal->sender_busy = false;
	pthread_mutex_unlock(&terminal->buf_mutex);

	terminal->on_data_sent(terminal->controller);
}

// -----------------------------------------------------------------------
static void on_async_write_complete(uv_write_t *req, int status)
{
	terminal_t *terminal = (terminal_t *) req->data;
	free(req);
	uv_timer_start(&terminal->timer_write, on_write_delay_timeout, 1, 0);
}

// -----------------------------------------------------------------------
int terminal_write(terminal_t *terminal, char c)
{
	int ret;

	LOGCHAR(L_TERM, "%s: ", "Terminal write start", c);

	pthread_mutex_lock(&terminal->buf_mutex);
	if (!terminal->sender_busy) {
		ret = 0;
	} else {
		ret = -1;
	}
	pthread_mutex_unlock(&terminal->buf_mutex);

	if (ret == 0) {
		if (terminal->client) {
			uv_write_t *req = (uv_write_t*) malloc(sizeof(uv_write_t));
			uv_buf_t buf = uv_buf_init(&c, 1);
			uv_req_set_data((uv_req_t*) req, terminal->client->data);
			uv_write((uv_write_t*) req, (uv_stream_t *) terminal->client, &buf, 1, on_async_write_complete);
		} else {
			uv_timer_start(&terminal->timer_write, on_write_delay_timeout, 1, 0);
		}
	}

	return ret;
}

// -----------------------------------------------------------------------
void terminal_reset(terminal_t *terminal)
{
	LOG(L_TERM, "Terminal reset");
	term_buf_reset(terminal);
}

// -----------------------------------------------------------------------
static void terminal_ioloop_teardown(terminal_t *terminal)
{
	if (terminal->client) {
		uv_close((uv_handle_t *) terminal->client, on_tcp_close);
	}
	uv_close((uv_handle_t *) &terminal->timer_write, NULL);
	uv_close((uv_handle_t *) &terminal->timer_read, NULL);
	uv_close((uv_handle_t *) &terminal->tcp_handle, NULL);
}

// -----------------------------------------------------------------------
void terminal_destroy(terminal_t *terminal)
{
	if (!terminal) return;

	LOG(L_TERM, "Terminal shutting down");

	terminal_ioloop_teardown(terminal);
}

// -----------------------------------------------------------------------
void terminal_free(terminal_t *terminal)
{
	if (!terminal) return;

	LOG(L_TERM, "Terminal freeing resources");

	free(terminal);
}

// -----------------------------------------------------------------------
static int terminal_ioloop_setup(terminal_t *terminal)
{
	int res;

	uv_tcp_init(ioloop, &terminal->tcp_handle);
	uv_handle_set_data((uv_handle_t*) &terminal->tcp_handle, (void*) terminal);
	struct sockaddr_in addr;

	uv_ip4_addr("127.0.0.1", terminal->port, &addr);
	res = uv_tcp_bind(&terminal->tcp_handle, (const struct sockaddr*) &addr, 0);
	if (res) {
		LOGERR("Terminal TCP bind to port %i error: %s", terminal->port, uv_strerror(res));
		return -1;
	}
	res = uv_listen((uv_stream_t*) &terminal->tcp_handle, 1, on_new_tcp_connection);
	if (res) {
		LOGERR("Terminal TCP listen on port %i error: %s", terminal->port, uv_strerror(res));
		return -1;
	}

	uv_timer_init(ioloop, &terminal->timer_write);
	uv_handle_set_data((uv_handle_t*) &terminal->timer_write, terminal);

	uv_timer_init(ioloop, &terminal->timer_read);
	uv_handle_set_data((uv_handle_t*) &terminal->timer_read, terminal);

	return 0;
}

// -----------------------------------------------------------------------
terminal_t * terminal_create(void *controller, on_data_received_cb cbr, on_data_sent_cb cbs, unsigned port, unsigned speed)
{
	terminal_t *terminal = calloc(1, sizeof(terminal_t));
	if (!terminal) {
		goto fail;
	}

	LOG(L_TERM, "Creating terminal: speed %i, TCP port %i", speed, port);

	term_buf_reset(terminal);
	terminal->client = NULL;
	terminal->port = port;
	terminal->speed = speed;
	terminal->on_data_received = cbr;
	terminal->on_data_sent = cbs;
	terminal->controller = controller;
	terminal->reset = terminal_reset;
	terminal->write = terminal_write;
	terminal->destroy = terminal_destroy;
	terminal->free = terminal_free;

	pthread_mutex_init(&terminal->buf_mutex, NULL);

	if (terminal_ioloop_setup(terminal)) {
		goto fail;
	}

	return terminal;

fail:
	terminal_free(terminal);
	return NULL;
}


// vim: tabstop=4 shiftwidth=4 autoindent
