//  Copyright (c) 2014 Jakub Filipowicz <jakubf@gmail.com>
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

#ifndef DEV_H
#define DEV_H

#include <pthread.h>

#include "cfg.h"

struct dev;

typedef int (*dev_setup_f)(struct dev *device, struct cfg_arg *args);
typedef void (*dev_close_f)(struct dev *device);
typedef void (*dev_reset_f)(struct dev *device);
typedef void (*dev_callback_f)(int arg);
typedef int (*dev_op_f)(struct dev *device, int op, char *buf, int addr, int count, int timeout);

enum dev_ops {
	DEV_OP_NONE = 0,
	DEV_OP_QUIT,
	DEV_OP_RESET,
	DEV_OP_READ,
	DEV_OP_WRITE,
	DEV_OP_PARK,
	DEV_OP_MAX,
};

struct dev {
	char *name;
	int size;
	dev_setup_f setup;
	dev_close_f close;
	dev_reset_f reset;
	dev_callback_f callback;
	dev_op_f operation;

	int op;
	char *buf;
	int addr;
	int count;
	int timeout;

	pthread_t worker_th;
	pthread_cond_t worker_cond;
	pthread_mutex_t worker_mutex;

};

extern struct dev * dev_create(char *name, struct cfg_arg *args, dev_callback_f callback);
extern void dev_close(struct dev *device);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
