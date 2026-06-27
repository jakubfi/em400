//  Copyright (c) 2026 Jakub Filipowicz <jakubf@gmail.com>
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
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>

#include "log.h"

#include "io/dev/rtclock.h"

// -----------------------------------------------------------------------
void rtclock_shutdown(em400_dev_t *dev)
{
	if (!dev) return;
	rtclock_t *rtclock = (rtclock_t *) dev;

	LOG(L_MECLO, "RTCLOCK shutting down");

	free(rtclock->prom_filename);
	free(rtclock);
}

// -----------------------------------------------------------------------
void rtclock_reset(em400_dev_t *dev)
{
	if (!dev) return;

	LOG(L_MECLO, "RTCLOCK reset");
}

// -----------------------------------------------------------------------
em400_dev_t * rtclock_create(const char *prom_filename)
{
	LOG(L_MECLO, "Creating (dummy) RTCLOCK device");

	rtclock_t *rtclock = calloc(1, sizeof(rtclock_t));
	if (!rtclock) {
		LOGERR("Failed to allocate memory for RTCLOCK device");
		return NULL;
	}

	if (prom_filename) {
		rtclock->prom_filename = strdup(prom_filename);
		if (!rtclock->prom_filename) {
			LOGERR("Failed to allocate memory for RTCLOCK PROM filename");
			free(rtclock);
			return NULL;
		}
	}

	rtclock->base.type = EM400_DEV_RTCLOCK;
	rtclock->base.slot_count = 0;
	rtclock->base.reset = NULL;
	rtclock->base.write = NULL;
	rtclock->base.shutdown = rtclock_shutdown;

	rtclock->base.can_eject = NULL;
	rtclock->base.load = NULL;
	rtclock->base.eject = NULL;
	rtclock->base.image = NULL;

	return (em400_dev_t *) rtclock;
}
