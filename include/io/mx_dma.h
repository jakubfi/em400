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

#ifndef MX_DMA_H
#define MX_DMA_H

#include <pthread.h>

#include "elst.h"
#include "log.h"
#include "io/mx_task.h"

enum mx_dma_action { MX_DMA_READ, MX_DMA_WRITE, MX_DMA_QUIT };

struct mx_dma {
	pthread_t thread;
	ELST list;
};

struct mx_dma * mx_dma_create();
void mx_dma_shutdown(struct mx_dma *dma);
int mx_dma_action(struct mx_dma *dma, struct mx_task *task, int action, unsigned nb, uint16_t addr, unsigned len, uint16_t *ptr);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
