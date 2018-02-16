//  Copyright (c) 2013-2015 Jakub Filipowicz <jakubf@gmail.com>
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

#include <pthread.h>

#include "io/mx_ev.h"
#include "io/mx_irq.h"
#include "io/mx_line.h"
#include "io/mx_task.h"
#include "io/mx_timer.h"

#include "log.h"

/*

 How devices work with MULTIX:

 .------------.            line             .--------.
 |            | .-------------------------. |        |
 |   MULTIX   |<         protocol          >| device |
 |            | `-------------------------' |        |
 `------------'                             `--------'

 * Line is a MULTIX abstract for connecting devices
 * Protocol is a set of methods to properly talk over a line to a device of a given type
 * Devices are brought to life by em400 based on emulator configuration file
 * Line and device configuration done by MULTIX' "setcfg" command is independet
   from devices configuration done in em400 configuration file
 * Above means that MERA-400 may for example try to configure and talk
   to a device which is not connected

*/

struct mx {
	int num;
	pthread_t main_th;						// main MULTIX thread
	pthread_t evproc_th;					// event processor thread

	int conf_set;							// configuration set flag

	struct mx_evt *evt;						// event table (internal MULTIX "interrupts")
	struct mx_irqq *irqq;					// interrupt request queue (to CPU)
	struct mx_timer *timer;					// MULTIX timer (500ms tick)

	struct mx_line lines[MX_LINE_MAX];		// physical lines

	struct mx_taskset *ts;

	// reset logic
	int state;
	pthread_mutex_t state_mutex;
	pthread_cond_t state_cond;
	int reset_ack;
	pthread_mutex_t reset_ack_mutex;
	pthread_cond_t reset_ack_cond;

	LOG_ID_DEF;
};

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
