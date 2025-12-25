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

#include <uv.h>
#include <pthread.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "terminal.h"

extern uv_loop_t *ioloop;
static char already_connected_message[] = "Terminal already connected. Bye.\n";

static void terminal_try_free(terminal_t *terminal);

// -----------------------------------------------------------------------
static void term_buf_reset(terminal_t *terminal)
{
	pthread_mutex_lock(&terminal->mutex);
	terminal->rdbuf_end = terminal->rdbuf + TERMINAL_BUF_SIZE-1;
	terminal->rdbuf_r = terminal->rdbuf;
	terminal->rdbuf_w = terminal->rdbuf;
	terminal->rdbuf_count = 0;
	terminal->sender_busy = false;
	pthread_mutex_unlock(&terminal->mutex);
}

// -----------------------------------------------------------------------
static int term_buf_get(terminal_t *terminal)
{
	int data;

	pthread_mutex_lock(&terminal->mutex);
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
	pthread_mutex_unlock(&terminal->mutex);

	return data;
}

// -----------------------------------------------------------------------
static int term_buf_append(terminal_t *terminal, char *c, int len)
{
	int ret = 0;

	pthread_mutex_lock(&terminal->mutex);
	while (len > 0) {
		if (terminal->rdbuf_count >= TERMINAL_BUF_SIZE) {
			ret = -1;
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
	pthread_mutex_unlock(&terminal->mutex);

	return ret;
}

// -----------------------------------------------------------------------
static void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
	buf->base = (char*) malloc(suggested_size);
	if (!buf->base) {
		buf->len = 0;
	} else {
		buf->len = suggested_size;
	}
}

// -----------------------------------------------------------------------
static void on_tcp_close(uv_handle_t* handle)
{
	terminal_t *terminal = (terminal_t *) uv_handle_get_data(handle);

	LOG(L_TERM, "Terminal TCP connection closed");
	terminal->client = NULL;
	terminal_try_free(terminal);
	free(handle);
}

// -----------------------------------------------------------------------
static void on_read_delay_timeout(uv_timer_t *handle)
{
	terminal_t *terminal = (terminal_t*) handle->data;

	int data = term_buf_get(terminal);
	if ((data > 0) && (terminal->controller) && (terminal->on_data_received)) {
		terminal->on_data_received(terminal->controller, data);
	}
	// push next character in buffer
	uv_timer_start(&terminal->timer_read, on_read_delay_timeout, terminal->delay_ms, 0);
}

// -----------------------------------------------------------------------
static void on_tcp_read(uv_stream_t *handle, ssize_t nread, const uv_buf_t *buf)
{
	int res = 0;
	terminal_t *terminal = (terminal_t *) handle->data;

	if (nread < 0) {
		LOG(L_TERM, "Terminal TCP Read error: %s", uv_err_name(nread));
		uv_close((uv_handle_t*) handle, on_tcp_close);
	} else {
		LOG(L_TERM, "Terminal read TCP data (%d bytes)", (int) nread);
		res = term_buf_append(terminal, buf->base, nread);
	}

	free(buf->base);

	if (res >= 0) {
		uv_timer_start(&terminal->timer_read, on_read_delay_timeout, terminal->delay_ms, 0);
	}
}

// -----------------------------------------------------------------------
static void on_tcp_reject_user(uv_write_t *req, int status)
{
	uv_close((uv_handle_t*) req->handle, on_tcp_close);
	free(req);
}

// -----------------------------------------------------------------------
static void on_new_tcp_connection(uv_stream_t *handle, int status)
{
	terminal_t *terminal = (terminal_t *) uv_handle_get_data((uv_handle_t *) handle);

	if (status < 0) {
		LOG(L_TERM, "Terminal new TCP connection error: %s", uv_strerror(status));
		return;
	}
	uv_tcp_t *client = (uv_tcp_t *) malloc(sizeof(uv_tcp_t));
	if (!client) {
		LOG(L_TERM, "Failed to allocate memory for TCP client handler");
		return;
	}
	int res = uv_tcp_init(ioloop, client);
	if (res) {
		free(client);
		LOGERR("Client connection initialization failed:", uv_strerror(res));
		return;
	}
	uv_handle_set_data((uv_handle_t*) client, terminal);
	terminal->open_handles++;

	if (uv_accept(handle, (uv_stream_t*) client)) {
		LOG(L_TERM, "Terminal failed to accept new TCP connection");
		uv_close((uv_handle_t*) client, on_tcp_close);
		return;
	}

	if (terminal->client) {
		LOG(L_TERM, "Terminal already connected, rejecting new connection");
		uv_write_t *req = (uv_write_t*) malloc(sizeof(uv_write_t));
		uv_buf_t buf = uv_buf_init(already_connected_message, strlen(already_connected_message));
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

	pthread_mutex_lock(&terminal->mutex);
	terminal->sender_busy = false;
	pthread_mutex_unlock(&terminal->mutex);

	if ((terminal->controller) && (terminal->on_data_sent)) {
		terminal->on_data_sent(terminal->controller);
	}
}

// -----------------------------------------------------------------------
static void on_async_write_complete(uv_write_t *req, int status)
{
	terminal_t *terminal = (terminal_t *) uv_req_get_data((uv_req_t*) req);
	free(req);
	uv_timer_start(&terminal->timer_write, on_write_delay_timeout, terminal->delay_ms, 0);
}

// -----------------------------------------------------------------------
int terminal_write(em400_dev_t *dev, char c)
{
	int ret;
	terminal_t *terminal = (terminal_t *) dev;

	LOGCHAR(L_TERM, "%s: ", "Terminal write start", c);

	pthread_mutex_lock(&terminal->mutex);
	if (!terminal->sender_busy) {
		terminal->sender_busy = true;
		terminal->wrbuf = c;
		ret = 0;
	} else {
		ret = -1;
	}
	pthread_mutex_unlock(&terminal->mutex);

	if (ret == 0) {
		if (terminal->client) {
			uv_write_t *req = (uv_write_t*) malloc(sizeof(uv_write_t));
			uv_buf_t buf = uv_buf_init(&terminal->wrbuf, 1);
			uv_req_set_data((uv_req_t*) req, terminal);
			int write_res = uv_write((uv_write_t*) req, (uv_stream_t *) terminal->client, &buf, 1, on_async_write_complete);
			if (write_res < 0) {
				free(req);
				uv_timer_start(&terminal->timer_write, on_write_delay_timeout, terminal->delay_ms, 0);
			}
		} else {
			uv_timer_start(&terminal->timer_write, on_write_delay_timeout, terminal->delay_ms, 0);
		}
	}

	return ret;
}

// -----------------------------------------------------------------------
void terminal_reset(em400_dev_t *dev)
{
	if (!dev) return;

	LOG(L_TERM, "Terminal reset");
	term_buf_reset((terminal_t *) dev);
}

// -----------------------------------------------------------------------
void terminal_free(em400_dev_t *dev)
{
	// TODO: method not required anymore, to be removed
}

// -----------------------------------------------------------------------
static void terminal_try_free(terminal_t *terminal)
{
	if (terminal->open_handles <= 0) {
		LOG(L_TERM, "No more open handles, terminal freeing resources");
		pthread_mutex_destroy(&terminal->mutex);
		free(terminal);
	}
}

// -----------------------------------------------------------------------
static void on_handle_close(uv_handle_t* handle)
{
	terminal_t *terminal = (terminal_t *) uv_handle_get_data(handle);
	terminal->open_handles--;
	terminal_try_free(terminal);
}

// -----------------------------------------------------------------------
static void terminal_ioloop_teardown(terminal_t *terminal)
{
	if ((terminal->client) && !uv_is_closing((uv_handle_t *) &terminal->client)){
		uv_close((uv_handle_t *) terminal->client, on_tcp_close);
	}
	if (!uv_is_closing((uv_handle_t *) &terminal->timer_write)) {
		uv_close((uv_handle_t *) &terminal->timer_write, on_handle_close);
	}
	if (!uv_is_closing((uv_handle_t *) &terminal->timer_read)) {
		uv_close((uv_handle_t *) &terminal->timer_read, on_handle_close);
	}
	if (!uv_is_closing((uv_handle_t *) &terminal->tcp_handle)) {
		uv_close((uv_handle_t *) &terminal->tcp_handle, on_handle_close);
	}
}

// -----------------------------------------------------------------------
void terminal_shutdown(em400_dev_t *dev)
{
	if (!dev) return;

	LOG(L_TERM, "Terminal shutting down");

	terminal_t *terminal = (terminal_t *) dev;

	terminal_ioloop_teardown(terminal);
}

// -----------------------------------------------------------------------
static int terminal_ioloop_setup(terminal_t *terminal)
{
	int res;

	res = uv_tcp_init(ioloop, &terminal->tcp_handle);
	if (res) {
		return LOGERR("Terminal TCP init error: %s", uv_strerror(res));
	}
	uv_handle_set_data((uv_handle_t*) &terminal->tcp_handle, (void*) terminal);
	terminal->open_handles++;

	struct sockaddr_in addr;
	res = uv_ip4_addr("127.0.0.1", terminal->port, &addr);
	if (res) {
		uv_close((uv_handle_t *) &terminal->tcp_handle, on_handle_close);
		return LOGERR("Terminal IPv4 address set error: %s", uv_strerror(res));
	}
	res = uv_tcp_bind(&terminal->tcp_handle, (const struct sockaddr*) &addr, 0);
	if (res) {
		uv_close((uv_handle_t *) &terminal->tcp_handle, on_handle_close);
		return LOGERR("Terminal TCP bind to port %i error: %s", terminal->port, uv_strerror(res));
	}
	res = uv_listen((uv_stream_t*) &terminal->tcp_handle, 1, on_new_tcp_connection);
	if (res) {
		uv_close((uv_handle_t *) &terminal->tcp_handle, on_handle_close);
		return LOGERR("Terminal TCP listen on port %i error: %s", terminal->port, uv_strerror(res));
	}

	res = uv_timer_init(ioloop, &terminal->timer_write);
	if (res) {
		uv_close((uv_handle_t *) &terminal->tcp_handle, on_handle_close);
		return LOGERR("Write timer setup error: %s", uv_strerror(res));
	}
	uv_handle_set_data((uv_handle_t*) &terminal->timer_write, terminal);
	terminal->open_handles++;

	res = uv_timer_init(ioloop, &terminal->timer_read);
	if (res) {
		uv_close((uv_handle_t *) &terminal->tcp_handle, on_handle_close);
		uv_close((uv_handle_t *) &terminal->timer_write, on_handle_close);
		return LOGERR("Read timer setup error: %s", uv_strerror(res));
	}
	uv_handle_set_data((uv_handle_t*) &terminal->timer_read, terminal);
	terminal->open_handles++;

	return E_OK;
}

// -----------------------------------------------------------------------
// TODO: generic device callback registration?
void terminal_register_callbacks(terminal_t * terminal, void *controller, on_data_received_cb recv_cb, on_data_sent_cb sent_cb)
{
	terminal->controller = controller;
	terminal->on_data_received = recv_cb;
	terminal->on_data_sent = sent_cb;
}

// -----------------------------------------------------------------------
em400_dev_t * terminal_create(unsigned port, unsigned speed)
{
	LOG(L_TERM, "Creating terminal: speed %i, TCP port %i", speed, port);

	if ((speed > 9600) || (speed < 150) || (speed % 150)) {
		LOGERR("Wrong terminal speed. Allowed values: 9600, 4800, 2400, 1200, 600, 300, 150");
		return NULL;
	}

	terminal_t *terminal = calloc(1, sizeof(terminal_t));
	if (!terminal) {
		return NULL;
	}

	if (pthread_mutex_init(&terminal->mutex, NULL)) {
		free(terminal);
		return NULL;
	}

	terminal->base.type = EM400_DEV_TERMINAL;
	terminal->base.reset = terminal_reset;
	terminal->base.write = terminal_write;
	terminal->base.shutdown = terminal_shutdown;

	term_buf_reset(terminal);
	terminal->client = NULL;
	terminal->port = port;
	// milisecond accuracy due to libuv limitations, 8 bits + start + stop
	terminal->delay_ms = roundf((float) ((8+2) * 1000) / speed);

	if (terminal_ioloop_setup(terminal) == E_ERR) {
		// may happen that setup did not initialize any handler
		// thus no callbacks would fire on handler close
		terminal_try_free(terminal);
		return NULL;
	}

	return (em400_dev_t *) terminal;
}


// vim: tabstop=4 shiftwidth=4 autoindent
