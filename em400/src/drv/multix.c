//  Copyright (c) 2013 Jakub Filipowicz <jakubf@gmail.com>
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

#include <inttypes.h>
#include <pthread.h>
#include <unistd.h>

#include "cfg.h"
#include "errors.h"
#include "io.h"
#include "drv/multix.h"

#include "debugger/log.h"

// -----------------------------------------------------------------------
void * drv_multix_thread(void *self)
{
	struct chan_t *ch = self;

	struct timespec ts;
/**/	ts.tv_sec = 0;
/**/	ts.tv_nsec = 1000000;

	while (!ch->finish) {
/**/		nanosleep(&ts, &ts);
	}
	pthread_exit(NULL);
}

// -----------------------------------------------------------------------
int drv_multix_init(void *self, struct cfg_arg_t *arg)
{
	struct chan_t *ch = self;
	drv_multix_reset(ch);
	pthread_create(&ch->thread, NULL, drv_multix_thread, ch);
	return E_OK;
}

// -----------------------------------------------------------------------
void drv_multix_shutdown(void *self)
{
	struct chan_t *ch = self;
	ch->finish = 1;
	pthread_join(ch->thread, NULL);
}

// -----------------------------------------------------------------------
void drv_multix_reset(void *self)
{
	struct chan_t *ch = self;
	ch->int_spec = 0;
	ch->int_mask = 0;
	ch->untransmitted = 0;
}

// -----------------------------------------------------------------------
int drv_multix_cmd(void *self, int dir, uint16_t n_arg, uint16_t *r_arg)
{
	return IO_OK;
}


// vim: tabstop=4
