//  Copyright (c) 2013-2018 Jakub Filipowicz <jakubf@gmail.com>
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
#include <inttypes.h>

#include "log.h"
#include "io/mx/mx.h"
#include "io/mx/line.h"
#include "io/mx/irq.h"

// -----------------------------------------------------------------------
int mx_dummy_attach(struct mx_line *line, uint16_t *cmd_data)
{
	pthread_mutex_lock(&line->status_mutex);
	line->status |= MX_LSTATE_ATTACHED;
	pthread_mutex_unlock(&line->status_mutex);

	return MX_IRQ_IDOLI;
}

// -----------------------------------------------------------------------
int mx_dummy_detach(struct mx_line *line, uint16_t *cmd_data)
{
	pthread_mutex_lock(&line->status_mutex);
	line->status &= ~MX_LSTATE_ATTACHED;
	pthread_mutex_unlock(&line->status_mutex);

	return MX_IRQ_IODLI;
}

// vim: tabstop=4 shiftwidth=4 autoindent
