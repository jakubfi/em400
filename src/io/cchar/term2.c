//  Copyright (c) 2013-2020, 2022 Jakub Filipowicz <jakubf@gmail.com>
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
#include "io/cchar/term2.h"

#include "log.h"
#include "cfg.h"

static uv_loop_t *loop;

static void * cchar_term2_evloop(void *ptr);
static void on_new_tcp_connection(uv_stream_t *handle, int status);
static void on_async_quit(uv_async_t *handle);
static void on_async_disconnect(uv_async_t *handle);
static void on_async_reset(uv_async_t *handle);
static void on_async_write(uv_async_t *handle);

// -----------------------------------------------------------------------
static void term_buf_reset(cchar_term2_t *term)
{
	pthread_mutex_lock(&term->rdbuf_mutex);
	term->rdbuf_end = term->rdbuf + TERM_BUF_SIZE-1;
	term->rdbuf_r = term->rdbuf;
	term->rdbuf_w = term->rdbuf;
	term->rdbuf_count = 0;
	term->rdbuf_ready = false;
	pthread_mutex_unlock(&term->rdbuf_mutex);
}

// -----------------------------------------------------------------------
static int term_buf_get(cchar_term2_t *term)
{
	int data;

	pthread_mutex_lock(&term->rdbuf_mutex);
	if (!term->rdbuf_ready) {
		data = -1;
		LOG(L_TERM, "buffer not ready");
	} else if (term->rdbuf_count <= 0) {
		data = -1;
		LOG(L_TERM, "no more data in buffer");
	} else {
		data = (unsigned char) *term->rdbuf_r;
		term->rdbuf_r++;
		term->rdbuf_count--;
		if (term->rdbuf_r > term->rdbuf_end) {
			term->rdbuf_r = term->rdbuf;
		}
		term->rdbuf_ready = false;
	}
	pthread_mutex_unlock(&term->rdbuf_mutex);

	return data;
}

// -----------------------------------------------------------------------
static int term_buf_append(cchar_term2_t *term, char *c, int len)
{
	int ret = 0;
	LOG(L_TERM, "Append %i bytes", len);
	pthread_mutex_lock(&term->rdbuf_mutex);
	while (len > 0) {
		if (term->rdbuf_count >= TERM_BUF_SIZE) {
			LOG(L_TERM, "no more space in the buffer");
			ret = -1;
			pthread_mutex_unlock(&term->mutex);
			break;
		} else {
			*term->rdbuf_w = *c;
			term->rdbuf_w++;
			term->rdbuf_count++;
			if (term->rdbuf_w > term->rdbuf_end) {
				term->rdbuf_w = term->rdbuf;
			}
		}
		c++;
		len--;
	}
	pthread_mutex_unlock(&term->rdbuf_mutex);

	LOG(L_TERM, "Read buffer after append: %i elements", term->rdbuf_count);

	return ret;
}

// -----------------------------------------------------------------------
#define __LOG_DATA(fmt, str, data) \
	if (isprint(data)) LOG(L_TERM, fmt "'%c'", str, data); \
	else LOG(L_TERM, fmt "#%02x", str, data);

// -----------------------------------------------------------------------
static int cchar_term2_setup(cchar_term2_t *term)
{
	static uv_tcp_t tcp_handle;
	uv_tcp_init(loop, &tcp_handle);
	uv_handle_set_data((uv_handle_t*) &tcp_handle, (void*) term);
	struct sockaddr_in addr;

	uv_ip4_addr("127.0.0.1", term->port, &addr);
	uv_tcp_bind(&tcp_handle, (const struct sockaddr*) &addr, 0);
	int r = uv_listen((uv_stream_t*) &tcp_handle, 1, on_new_tcp_connection);
	if (r) {
		LOG(L_TERM, "TCP Listen error: %s", uv_strerror(r));
		return -1;
	}

	uv_async_init(loop, &term->async_quit, on_async_quit);
	uv_handle_set_data((uv_handle_t*) &term->async_quit, term);

	uv_async_init(loop, &term->async_disconnect, on_async_disconnect);
	uv_handle_set_data((uv_handle_t*) &term->async_disconnect, term);

	uv_async_init(loop, &term->async_reset, on_async_reset);
	uv_handle_set_data((uv_handle_t*) &term->async_reset, term);

	uv_async_init(loop, &term->async_write, on_async_write);
	uv_handle_set_data((uv_handle_t*) &term->async_write, term);

	uv_timer_init(loop, &term->timer_dir_switch);
	uv_handle_set_data((uv_handle_t*) &term->timer_dir_switch, term);

	uv_timer_init(loop, &term->timer_write);
	uv_handle_set_data((uv_handle_t*) &term->timer_write, term);

	uv_timer_init(loop, &term->timer_read);
	uv_handle_set_data((uv_handle_t*) &term->timer_read, term);

	return 0;
}

// -----------------------------------------------------------------------
struct cchar_unit_proto_t * cchar_term2_create(em400_cfg *cfg, int ch_num, int dev_num)
{
	cchar_term2_t *term = (cchar_term2_t*) calloc(1, sizeof(cchar_term2_t));
	if (!term) {
		LOGERR("Failed to allocate memory for terminal: %i.%i", ch_num, dev_num);
		return NULL;
	}

	term->speed = cfg_fgetint(cfg, "dev%i.%i:speed", ch_num, dev_num);
	term->port = cfg_fgetint(cfg, "dev%i.%i:port", ch_num, dev_num);
	term->state = STATE_IDLE;

	term_buf_reset(term);

	LOG(L_TERM, "Creating terminal: speed: %i, port: %i", term->speed, term->port);

	if (!term->port) {
		LOGERR("TCP terminal needs port to be set.");
		goto fail;
	}

	loop = uv_default_loop();

	if (cchar_term2_setup(term)) goto fail;

	if (pthread_create(&term->thread, NULL, cchar_term2_evloop, term)) {
		LOGERR("Failed to spawn main I/O tester thread.");
		goto fail;
	}

	pthread_setname_np(term->thread, "UVTERM");

	return (struct cchar_unit_proto_t *) term;

fail:
	cchar_term2_shutdown((struct cchar_unit_proto_t*) term);
	return NULL;
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
	cchar_term2_t *term = (cchar_term2_t *) handle->data;

	LOG(L_TERM, "Close TCP connection");
	free(handle);
	term->client = NULL;
}

// -----------------------------------------------------------------------
static void on_read_delay_timeout(uv_timer_t *handle)
{
	LOG(L_TERM, "READ delay complete");
	cchar_term2_t *term = (cchar_term2_t*) handle->data;

	pthread_mutex_lock(&term->mutex);
	if (term->rdbuf_count > 0) {
		term->rdbuf_ready = true;
		atomic_store_explicit(&term->intspec, CCHAR_TERM2_INT_READY, memory_order_release);
		cchar_int_trigger(term->proto.chan);
		uv_timer_start(&term->timer_write, on_read_delay_timeout, 1, 0);
	}
	pthread_mutex_unlock(&term->mutex);

}

// -----------------------------------------------------------------------
static void on_tcp_read(uv_stream_t *handle, ssize_t nread, const uv_buf_t *buf)
{
	int res;
	cchar_term2_t *term = (cchar_term2_t *) handle->data;

	if (nread < 0) {
		if (nread != UV_EOF) {
			LOG(L_TERM, "TCP Read error: %s", uv_err_name(nread));
		} else {
			uv_close((uv_handle_t*) handle, on_tcp_close);
		}
	} else {
		LOG(L_TERM, "Read TCP data (%d bytes)", nread);
		res = term_buf_append(term, buf->base, nread);
	}

	free(buf->base);

	if (res >= 0) {
		uv_timer_start(&term->timer_write, on_read_delay_timeout, 1, 0);
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
		LOG(L_TERM, "New connection error: %s", uv_strerror(status));
		return;
	}
	uv_tcp_t *client = (uv_tcp_t *) malloc(sizeof(uv_tcp_t));

	uv_tcp_init(loop, client);
	uv_handle_set_data((uv_handle_t*) client, handle->data);

	if (uv_accept(handle, (uv_stream_t*) client)) {
		LOG(L_TERM, "Failed to accept new TCP connection");
		uv_close((uv_handle_t*) client, on_tcp_close);
		return;
	}

	cchar_term2_t *term = (cchar_term2_t *) handle->data;

	if (term->client) {
		LOG(L_TERM, "Rejecting connection to already connected terminal");
		uv_write_t *req = (uv_write_t*) malloc(sizeof(uv_write_t));
		static char message[] = "Terminal already connected. Bye.\n";
		uv_buf_t buf = uv_buf_init(message, strlen(message));
		uv_write((uv_write_t*) req, (uv_stream_t *) client, &buf, 1, on_tcp_reject_user);
		return;
	}

	LOG(L_TERM, "Accepted new TCP connection");
	term->client = client;
	uv_read_start((uv_stream_t*) client, alloc_buffer, on_tcp_read);
}

// -----------------------------------------------------------------------
static void on_async_quit(uv_async_t *handle)
{
	LOG(L_TERM, "QUIT received");
	uv_stop(loop);
}

// -----------------------------------------------------------------------
static void on_dir_switch_timeout(uv_timer_t *handle)
{
	cchar_term2_t *term = (cchar_term2_t*) handle->data;

	pthread_mutex_lock(&term->mutex);
	term->state = STATE_WRITE_RDY;
	atomic_store_explicit(&term->intspec, CCHAR_TERM2_INT_READY, memory_order_release);
	pthread_mutex_unlock(&term->mutex);

	cchar_int_trigger(term->proto.chan);
}

// -----------------------------------------------------------------------
static void on_async_disconnect(uv_async_t *handle)
{
	cchar_term2_t *term = (cchar_term2_t*) handle->data;

	pthread_mutex_lock(&term->mutex);
	term->state = STATE_IDLE;
	pthread_mutex_unlock(&term->mutex);

	term_buf_reset(term);
}

// -----------------------------------------------------------------------
static void on_async_reset(uv_async_t *handle)
{
	cchar_term2_t *term = (cchar_term2_t*) handle->data;

	pthread_mutex_lock(&term->mutex);
	term->state = STATE_IDLE;
	pthread_mutex_unlock(&term->mutex);

	term_buf_reset(term);
}

// -----------------------------------------------------------------------
static void on_write_delay_timeout(uv_timer_t *handle)
{
	LOG(L_TERM, "WRITE delay complete");
	cchar_term2_t *term = (cchar_term2_t*) handle->data;

	pthread_mutex_lock(&term->mutex);
	term->state = STATE_WRITE_RDY;
	atomic_store_explicit(&term->intspec, CCHAR_TERM2_INT_READY, memory_order_release);
	pthread_mutex_unlock(&term->mutex);

	cchar_int_trigger(term->proto.chan);
}

// -----------------------------------------------------------------------
static void on_async_write_complete(uv_write_t *req, int status)
{
	LOG(L_TERM, "WRITE complete, delaying");
	cchar_term2_t *term = (cchar_term2_t *) req->data;
	free(req);
	uv_timer_start(&term->timer_write, on_write_delay_timeout, 1, 0);
}

// -----------------------------------------------------------------------
static void on_async_write(uv_async_t *handle)
{
	LOG(L_TERM, "WRITE received");
	cchar_term2_t *term = (cchar_term2_t*) handle->data;

	pthread_mutex_lock(&term->mutex);
	int state = term->state;
	pthread_mutex_unlock(&term->mutex);

	switch (state) {
		case STATE_WRITING:
			if (term->client) {
				__LOG_DATA("%s: ", "WRITE to client", term->buf_wr)
				uv_write_t *req = (uv_write_t*) malloc(sizeof(uv_write_t));
				uv_buf_t buf = uv_buf_init(&term->buf_wr, 1);
				uv_req_set_data((uv_req_t*) req, handle->data);
				uv_write((uv_write_t*) req, (uv_stream_t *) term->client, &buf, 1, on_async_write_complete);
			} else {
				LOG(L_TERM, "No client connected, write lost");
				uv_timer_start(&term->timer_write, on_write_delay_timeout, 1, 0);
			}
			break;
		case STATE_IDLE:
		case STATE_READ:
			LOG(L_TERM, "Switching to WRITE mode");
			uv_timer_start(&term->timer_dir_switch, on_dir_switch_timeout, 1, 0);
			break;
		default:
			LOG(L_TERM, "Previous WRITE active");
			break;
	}
}

// -----------------------------------------------------------------------
static void * cchar_term2_evloop(void *ptr)
{
	LOG(L_TERM, "Starting UV loop");
	uv_run(loop, UV_RUN_DEFAULT);
	LOG(L_TERM, "Exited UV loop");

	uv_loop_close(loop);

	pthread_exit(NULL);
}

// -----------------------------------------------------------------------
void cchar_term2_shutdown(struct cchar_unit_proto_t *unit)
{
	LOG(L_TERM, "UV Terminal shutting down");
	cchar_term2_t *term = (cchar_term2_t*) unit;
	if (!term) return;

	uv_async_send(&term->async_quit);
	pthread_join(term->thread, NULL);
	free(term);
}

// -----------------------------------------------------------------------
void cchar_term2_reset(struct cchar_unit_proto_t *unit)
{
	LOG(L_TERM, "Command: reset");
	cchar_term2_t *term = (cchar_term2_t*) unit;
	uv_async_send(&term->async_reset);
}

// -----------------------------------------------------------------------
int cchar_term2_intspec(struct cchar_unit_proto_t *unit)
{
	cchar_term2_t *term = (cchar_term2_t*) unit;

	int spec = atomic_load_explicit(&term->intspec, memory_order_acquire);
	atomic_store_explicit(&term->intspec, CCHAR_TERM2_INT_OUTDATED, memory_order_release);

	return spec;
}

// -----------------------------------------------------------------------
bool cchar_term2_has_interrupt(struct cchar_unit_proto_t *unit)
{
	cchar_term2_t *term = (cchar_term2_t*) unit;
	return atomic_load_explicit(&term->intspec, memory_order_acquire) ? true : false;
}

// -----------------------------------------------------------------------
static int cchar_term2_read(cchar_term2_t *term, uint16_t *r_arg)
{
	int res;

	int data = term_buf_get(term);
	pthread_mutex_lock(&term->mutex);
	if (data >= 0) {
		*r_arg = (*r_arg & 0xff00) | (data &0xff);
		res = IO_OK;
	} else if (term->state != STATE_WRITING) {
		term->state = STATE_READ;
		res = IO_EN;
	} else {
		res = IO_EN;
	}
	pthread_mutex_unlock(&term->mutex);

	if (res == IO_OK) {
		__LOG_DATA("%s: ", "READ ready, received: ", data & 0xff)
	} else {
		LOG(L_TERM, "READ not ready");
	}

	return res;
}

// -----------------------------------------------------------------------
static int cchar_term2_write(cchar_term2_t *term, uint16_t *r_arg)
{
	static const char *log_msgs[] = {
		"WRITE ready, sending",
		"WRITE busy, not sending",
		"transmitter not ready, not sending",
	};
	const char *log_msg = NULL;
	bool wakeup_transmitter = false;
	int res = IO_EN;

	char data = *r_arg & 0xff;

	pthread_mutex_lock(&term->mutex);
	if (term->state == STATE_WRITE_RDY) {
		term->buf_wr = data;
		term->state = STATE_WRITING;
		wakeup_transmitter = true;
		log_msg = log_msgs[0];
		res = IO_OK;
	} else if (term->state == STATE_WRITING) {
		log_msg = log_msgs[1];
	} else { // IDLE or READ
		log_msg = log_msgs[2];
		wakeup_transmitter = true;
	}
	pthread_mutex_unlock(&term->mutex);

	__LOG_DATA("%s: ", log_msg, data);
	if (wakeup_transmitter) uv_async_send(&term->async_write);

	return res;
}

// -----------------------------------------------------------------------
static int cchar_term2_disconnect(cchar_term2_t *term)
{
	LOG(L_TERM, "Command: disconnect");

	uv_async_send(&term->async_disconnect);

	return IO_OK;
}

// -----------------------------------------------------------------------
int cchar_term2_cmd(struct cchar_unit_proto_t *unit, int dir, int cmd, uint16_t *r_arg)
{
	cchar_term2_t *term = (cchar_term2_t*) unit;

	if (dir == IO_IN) {
		switch (cmd) {
			case CCHAR_TERM2_CMD_SPU:
				LOG(L_TERM, "Command: SPU");
				return IO_OK;
			case CCHAR_TERM2_CMD_READ:
				return cchar_term2_read(term, r_arg);
			default:
				LOG(L_TERM, "Unknown IN command: %i", cmd);
				return IO_NO;
		}
	} else {
		switch (cmd) {
			case CCHAR_TERM2_CMD_RESET:
				cchar_term2_reset(unit);
				return IO_OK;
			case CCHAR_TERM2_CMD_DISCONNECT:
				return cchar_term2_disconnect(term);
			case CCHAR_TERM2_CMD_WRITE:
				return cchar_term2_write(term, r_arg);
			default:
				LOG(L_TERM, "Unknown OU command: %i", cmd);
				return IO_NO;
		}
	}
}

// vim: tabstop=4 shiftwidth=4 autoindent
