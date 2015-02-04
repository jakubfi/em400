//  Copyright (c) 2013-2014 Jakub Filipowicz <jakubf@gmail.com>
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

#ifndef MX_H
#define MX_H

#define MX_LINE_MAX 32

#include <inttypes.h>
#include <pthread.h>

#include "io/chan.h"
#include "io/mx_intr.h"
#include "io/mx_cmd.h"
#include "io/mx_task.h"
#include "io/mx_cf.h"
#include "io/dev.h"

struct mx_line {
	int used;					// line is used (1=physical 2=logical)
	int dir;					// data direction
	int type;					// line type
	int attached;				// line is attached
	struct mx_proto *proto;

	// actual device
	struct dev *device;

	// winchester and flopy
	int dtype;					// disk type
	int format_protect;			// disk is format protected

	// MT
	int formatter;				// formatter number

	// line tasks management
	uint16_t addr;
};

// MULTIX
struct mx_chan {
	struct chan proto;

	// interrupt queue
	int intrq_len;
	struct mx_intr *intr_head;
	struct mx_intr *intr_tail;
	pthread_mutex_t intr_mutex;

	// task manager
	struct mx_task_line task[MX_TASK_MAX];
	int task_em;
	pthread_cond_t task_cond;
	pthread_mutex_t task_mutex;
	pthread_t task_th;

	// command receiver
	pthread_t cmd_recv_th;
	pthread_cond_t cmd_recv_cond;
	pthread_mutex_t cmd_recv_mutex;
	int cmd_recv;
	int cmd_recv_llinen;
	uint16_t cmd_recv_addr;

	// lines configuration
	int conf_set;
	pthread_mutex_t conf_mutex;
	struct mx_line pline[MX_LINE_MAX];
	struct mx_line *lline[MX_LINE_MAX];
	struct mx_dev *dev;

};

struct chan * mx_create(struct cfg_unit *units);
void mx_shutdown(struct chan *chan);
void mx_reset(struct chan *chan);
int mx_cmd(struct chan *chan, int dir, uint16_t n_arg, uint16_t *r_arg);

void mx_task_clear_all(struct mx_chan *chan);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
