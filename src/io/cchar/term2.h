//  Copyright (c) 2012-2013 Jakub Filipowicz <jakubf@gmail.com>
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

#ifndef __CCHAR_TERM2_H__
#define __CCHAR_TERM2_H__

#include <stdbool.h>
#include <pthread.h>
#include <uv.h>

#include "io/cchar/cchar.h"
#include "cfg.h"

#define TERM_BUF_SIZE 10

typedef struct cchar_term2_s {
	struct cchar_unit_proto_t proto;
	pthread_t thread;
	pthread_mutex_t mutex;
	int port;
	int speed;
	int intspec;
	int state;
	char buf_wr;

	pthread_mutex_t rdbuf_mutex;
	char rdbuf[TERM_BUF_SIZE];
	char *rdbuf_r;
	char *rdbuf_w;
	char *rdbuf_end;
	int rdbuf_count;
	bool rdbuf_ready;

	uv_tcp_t *client;
	uv_async_t async_quit;
	uv_async_t async_write;
	uv_async_t async_disconnect;
	uv_async_t async_reset;
	uv_timer_t timer_dir_switch;
	uv_timer_t timer_write;
	uv_timer_t timer_read;
} cchar_term2_t;

enum cchar_term2_sates {
	STATE_IDLE, STATE_WRITE_RDY, STATE_WRITING, STATE_READ
};

enum cchar_term2_commands {
	// OU
	CCHAR_TERM2_CMD_RESET		= 0b100000, // reset
	CCHAR_TERM2_CMD_DISCONNECT	= 0b101000, // disconnect device (soft reset)
	CCHAR_TERM2_CMD_WRITE		= 0b110000, // write
	// IN
	CCHAR_TERM2_CMD_SPU			= 0b100000, // check device
	CCHAR_TERM2_CMD_READ		= 0b101000, // read
};

enum cchar_term2_interrupts {
	CCHAR_TERM2_INT_OUTDATED	= 0, // interrupt out of date
	CCHAR_TERM2_INT_READY		= 1, // ready again
	CCHAR_TERM2_INT_TOO_SLOW	= 5, // transmission too slow
};

struct cchar_unit_proto_t * cchar_term2_create(em400_cfg *cfg, int ch_num, int dev_num);
void cchar_term2_shutdown(struct cchar_unit_proto_t *unit);
void cchar_term2_reset(struct cchar_unit_proto_t *unit);
int cchar_term2_cmd(struct cchar_unit_proto_t *unit, int dir, int cmd, uint16_t *r_arg);
int cchar_term2_intspec(struct cchar_unit_proto_t *unit);
bool cchar_term2_has_interrupt(struct cchar_unit_proto_t *unit);


#endif

// vim: tabstop=4 shiftwidth=4 autoindent
