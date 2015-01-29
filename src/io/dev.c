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

#include <stdlib.h>
#include <strings.h>
#include <pthread.h>

#include "errors.h"
#include "io/dev.h"
#include "io/dev_winch.h"

static void * dev_worker(void *ptr);

struct dev dev_map[] = {
	{ "winchester", sizeof(struct dev_winch),
		dev_winch_setup,
		dev_winch_close,
		dev_winch_reset,
	},
	{ NULL }
};

// -----------------------------------------------------------------------
struct dev * dev_create(char *name, struct cfg_arg *args, dev_callback_f callback)
{
	int res;
	struct dev *d = dev_map;
	struct dev *device = NULL;

	// find dev proto
	while (d && d->name) {
		if (!strcasecmp(name, d->name)) {
			break;
		}
		d++;
	}

	// did we find anything?
	if (!d || !d->name) {
		gerr = E_IO_UNIT_UNKNOWN;
		goto cleanup;
	}

	// alloc for specific device
	device = calloc(1, d->size);
	if (!device) {
		gerr = E_ALLOC;
		goto cleanup;
	} 

	// setup general device stuff
	device->setup = d->setup;
	device->close = d->close;
	device->reset = d->reset;
	device->callback = callback;

	pthread_mutex_init(&device->worker_mutex, NULL);
	pthread_cond_init(&device->worker_cond, NULL);

	if (pthread_create(&device->worker_th, NULL, dev_worker, device)) {
		gerr = E_THREAD;
		goto cleanup;
	}

	// setup device-specific stupp
	res = device->setup(device, args);
	if (res != E_OK) {
		gerr = res;
		goto cleanup;
	}

	return device;

cleanup:
	dev_close(device);
	return NULL;
}

// -----------------------------------------------------------------------
void dev_close(struct dev *device)
{
	if (!device) return;

	pthread_mutex_lock(&device->worker_mutex);
	device->close(device);
	device->op = DEV_OP_QUIT;
	pthread_cond_signal(&device->worker_cond);
	pthread_mutex_unlock(&device->worker_mutex);
	pthread_join(device->worker_th, NULL);

	free(device);
}

// -----------------------------------------------------------------------
void dev_reset(struct dev *device)
{
	if (!device) return;

	device->reset(device);
	pthread_mutex_lock(&device->worker_mutex);
	device->op = DEV_OP_RESET;
	pthread_cond_signal(&device->worker_cond);
	pthread_mutex_unlock(&device->worker_mutex);
}

// -----------------------------------------------------------------------
void dev_op(struct dev *device, int op, char *buf, int addr, int count, int timeout)
{
	pthread_mutex_lock(&device->worker_mutex);

	device->op = op;
	device->buf = buf;
	device->addr = addr;
	device->count = count;

	pthread_cond_signal(&device->worker_cond);
	pthread_mutex_unlock(&device->worker_mutex);
}

// -----------------------------------------------------------------------
static void * dev_worker(void *ptr)
{
	int res;
	struct dev *device = (struct dev *) ptr;

	while (1) {
		pthread_mutex_lock(&device->worker_mutex);

		while (device->op == DEV_OP_NONE) {
			pthread_cond_wait(&device->worker_cond, &device->worker_mutex);
		}

		if (device->op == DEV_OP_QUIT) {
			pthread_mutex_unlock(&device->worker_mutex);
			break;
		}

		if (device->op != DEV_OP_RESET) {
			res = device->operation(device, device->op, device->buf, device->addr, device->count, device->timeout);
			if (res >= 0) {
				device->callback(res);
			}
		}

		device->op = DEV_OP_NONE;
		device->buf = NULL;
		device->addr = 0;
		device->count = 0;
		device->timeout = 0;

		pthread_mutex_unlock(&device->worker_mutex);
	}

	pthread_exit(NULL);
}

// vim: tabstop=4 shiftwidth=4 autoindent
