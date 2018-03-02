//  Copyright (c) 2018 Jakub Filipowicz <jakubf@gmail.com>
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
#include <pthread.h>

#include "log.h"
#include "io/io.h"
#include "io/mx_dma.h"
#include "io/mx_task.h"

struct mx_dma_req {
	int action;
	unsigned nb;
	uint16_t addr;
	unsigned len;
	uint16_t *ptr;
	struct mx_task *task;
};

static void * mx_dma_main(void *ptr);

// -----------------------------------------------------------------------
struct mx_dma * mx_dma_create()
{
	struct mx_dma *dma = calloc(1, sizeof(struct mx_dma));
	if (!dma) {
		log_err("Memory allocation error.");
		goto cleanup;
	}

	dma->list = elst_create(32);
	if (!dma->list) {
		log_err("Memory allocation error.");
		goto cleanup;
	}

	if (pthread_create(&dma->thread, NULL, mx_dma_main, dma)) {
		log_err("Failed to spawn DMA thread.");
		goto cleanup;
	}

	return dma;

cleanup:
	if (dma) elst_destroy(dma->list);
	free(dma);
	return NULL;
}

// -----------------------------------------------------------------------
void mx_dma_shutdown(struct mx_dma *dma)
{
	if (!dma) return;

	mx_dma_action(dma, NULL, MX_DMA_QUIT, 0, 0, 0, NULL);
	pthread_join(dma->thread, NULL);

	elst_destroy(dma->list);
	free(dma);
}

// -----------------------------------------------------------------------
int mx_dma_action(struct mx_dma *dma, struct mx_task *task, int action, unsigned nb, uint16_t addr, unsigned len, uint16_t *ptr)
{
	struct mx_dma_req *req = malloc(sizeof(struct mx_dma_req));
	if (!req) {
		return -1;
	}

	req->task = task;
	req->action = action;
	req->nb = nb;
	req->addr = addr;
	req->len = len;
	req->ptr = ptr;

	int res = elst_append(dma->list, req);
	if (res < 0) {
		return -2;
	}

	return 0;
}

// -----------------------------------------------------------------------
static void * mx_dma_main(void *ptr)
{
	int result;
	struct mx_dma *dma = ptr;

	while (1) {
		struct mx_dma_req *req = elst_wait_pop(dma->list);
		if (!req) {
			break;
		}

		if (req->action == MX_DMA_READ) {
			result = io_mem_mget(req->nb, req->addr, req->ptr, req->len);
		} else if (req->action == MX_DMA_WRITE) {
			result = io_mem_mput(req->nb, req->addr, req->ptr, req->len);
		} else if (req->action == MX_DMA_QUIT) {
			break;
		} else {
			// TODO: unknown request
		}

		// TODO: przenieść zadanie do kolejki zadań do wykonania
	}

	pthread_exit(NULL);
}

// vim: tabstop=4 shiftwidth=4 autoindent
