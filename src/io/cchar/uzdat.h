//  Copyright (c) 2012-2024 Jakub Filipowicz <jakubf@gmail.com>
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
#include "io/cchar/terminal.h"
#include "cfg.h"


typedef struct uzdat_s {
	struct cchar_unit_proto_t proto;
	pthread_t thread;
	pthread_mutex_t mutex;
	int intspec;
	int state;
	int dir;
	bool xfer_busy;
	char buf_wr;
	int buf_rd;

	terminal_t *terminal;

	uv_async_t async_quit;
	uv_async_t async_write;
	uv_async_t async_disconnect;
	uv_async_t async_reset;
	uv_async_t async_switch_transmit;
	uv_timer_t timer_switch_transmit;
} uzdat_t;

enum uzdat_sates {
	UZDAT_STATE_OFF,
	UZDAT_STATE_OK,
	UZDAT_STATE_EN
};

enum uzdat_dirs {
	UZDAT_DIR_NONE,
	UZDAT_DIR_IN,
	UZDAT_DIR_OUT
};

enum uzdat_commands {
	// OU
	UZDAT_CMD_RESET			= 0b100000, // reset
	UZDAT_CMD_DISCONNECT	= 0b101000, // disconnect device (soft reset)
	UZDAT_CMD_WRITE			= 0b110000, // write
	// IN
	UZDAT_CMD_SPU			= 0b100000, // check device presence
	UZDAT_CMD_READ			= 0b101000, // read
};

enum cchar_uzdat_interrupts {
	UZDAT_INT_OUTDATED	= 0, // interrupt out of date
	UZDAT_INT_READY		= 1, // ready again
	UZDAT_INT_TOO_SLOW	= 5, // transmission too slow
};

struct cchar_unit_proto_t * uzdat_create(em400_cfg *cfg, int ch_num, int dev_num);
void uzdat_shutdown(struct cchar_unit_proto_t *unit);
void uzdat_reset(struct cchar_unit_proto_t *unit);
int uzdat_cmd(struct cchar_unit_proto_t *unit, int dir, int cmd, uint16_t *r_arg);
int uzdat_intspec(struct cchar_unit_proto_t *unit);
bool uzdat_has_interrupt(struct cchar_unit_proto_t *unit);


#endif

// vim: tabstop=4 shiftwidth=4 autoindent
