//  Copyright (c) 2012-2018 Jakub Filipowicz <jakubf@gmail.com>
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

#include <inttypes.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdatomic.h>

#include "utils/elst.h"
#include "io/mx/cmds.h"

#define MX_LINE_CNT 32
#define MX_LINE_BUF_SIZE 512

struct mx_line;
struct mx;

typedef int (*mx_proto_init_fun)(struct mx_line *pline, uint16_t *data);
typedef void (*mx_proto_destroy_fun)(struct mx_line *pline);
typedef int (*mx_proto_cmd_fun)(struct mx_line *pline, uint16_t *data);
typedef int (*mx_proto_decode_fun)(uint16_t *data, void *proto_data);
typedef void (*mx_proto_encode_fun)(uint16_t *data, void *proto_data);

struct mx_cmd {
	const int input_flen;
	const int output_flen;
	const mx_proto_decode_fun decode;
	const mx_proto_encode_fun encode;
	const mx_proto_cmd_fun run;
};

struct mx_proto {
	const char *name;
	const uint16_t dir;
	const int phy_types[3]; // never more than 2 + 1 (terminator) positions
	const mx_proto_init_fun init;
	const mx_proto_destroy_fun destroy;
	const struct mx_cmd cmd[MX_CMD_CNT];
};

struct mx_line {
	struct mx *multix;				// multix line is connected to

	int used;						// physical line is used
	int dir;						// physical line direction
	int type;						// physical line type
	uint32_t status;				// line status (32-bit due to additional em400 flags, only the least sig. word is sent to MERA-400)
	pthread_mutex_t status_mutex;	// line status mutex

	int phy_n;						// physical line number
	int log_n;						// logical line number

	const struct dev_drv *dev;		// device driver
	void *dev_data;					// device data

	ELST protoq;					// protocol event queue
	const struct mx_proto *proto;	// protocol driver
	void *proto_data;				// protocol private data
	pthread_t proto_th;				// protocol thread
	uint8_t buf[MX_LINE_BUF_SIZE];	// line transmission data buffer

	ELST statusq;					// status event queue
	pthread_t status_th;			// status thread
};

struct mx {
	int chnum;						// Multix' channel number
	atomic_int state;				// multix state (uninitialized, initialized, configured)

	ELST eventq;					// event queue
	pthread_t ev_thread;			// event processor thread

	ELST intq;						// interrupt queue
	uint16_t intspec;				// specification of the interrupt reported to the CPU
	pthread_mutex_t int_mutex;		// mutex guarding interrupt specification

	struct mx_line plines[MX_LINE_CNT];  // physical lines
	struct mx_line *llines[MX_LINE_CNT]; // logical lines (mapping to physical lines)
};

int mx_int_enqueue(struct mx *multix, int intr, int line);
bool mx_mem_read(struct mx *multix, int nb, uint16_t addr, uint16_t *data, int len);
bool mx_mem_write(struct mx *multix, int nb, uint16_t addr, uint16_t *data, int len);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
