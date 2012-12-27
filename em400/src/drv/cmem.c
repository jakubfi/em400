//  Copyright (c) 2012 Jakub Filipowicz <jakubf@gmail.com>
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

#include <pthread.h>
#include <unistd.h>

#include "errors.h"
#include "io.h"
#include "drv/cmem.h"

// -----------------------------------------------------------------------
void * drv_cmem_thread(void *ptr)
{
	struct chan_t *chan = ptr;
	while (!chan->finish) {
		sleep(1);
	}
	pthread_exit(NULL);
}

// -----------------------------------------------------------------------
int drv_cmem_init(struct chan_t *ch)
{
	drv_cmem_reset(ch);
	pthread_create(&ch->thread, NULL, drv_cmem_thread, ch);
	return E_OK;
}

// -----------------------------------------------------------------------
void drv_cmem_shutdown(struct chan_t *ch)
{
	ch->finish = 1;
	pthread_join(ch->thread, NULL);
}

// -----------------------------------------------------------------------
void drv_cmem_reset(struct chan_t *ch)
{
	ch->int_spec = 0;
	ch->int_mask = 0;
	ch->dev_alloc = 0;
}

// -----------------------------------------------------------------------
int drv_cmem_cmd(struct chan_t *ch, int dir, int unit, int cmd, int r)
{
	return IO_EN;
}

// vim: tabstop=4
