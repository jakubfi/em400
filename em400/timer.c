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

#include <sys/time.h>
#include <stdio.h>
#include <features.h>
#include <signal.h>
#include <time.h>

#include "errors.h"
#include "timer.h"
#include "registers.h"

// -----------------------------------------------------------------------
void _mjc400_timer_interrupt_sig(int signum, siginfo_t *si, void *ctx)
{
	RZ_5sb;
}

// -----------------------------------------------------------------------
int mjc400_timer_start()
{
	struct sigaction sa;
	struct sigevent se;
	struct itimerspec its;

	timer_t timer;

    sa.sa_flags = SA_SIGINFO | SA_RESTART;
    sa.sa_sigaction = _mjc400_timer_interrupt_sig;

    if (sigemptyset(&sa.sa_mask) != 0) {
		return E_TIMER_SIGNAL;
	}

    if (sigaction(MJC400_TIMER_SIG, &sa, NULL) != 0) {
        return E_TIMER_SIGNAL;
    }

    se.sigev_notify = SIGEV_SIGNAL;
    se.sigev_signo = MJC400_TIMER_SIG;
    se.sigev_value.sival_ptr = &timer;

    if (timer_create(CLOCK_REALTIME, &se, &timer) != 0) {
		return E_TIMER_CREATE;
	}

    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = MJC400_TIMER * 1000000;
    its.it_value.tv_sec = 0;
    its.it_value.tv_nsec = MJC400_TIMER * 1000000;

    if (timer_settime(&timer, 0, &its, NULL) != 0) {
		return E_TIMER_SET;
	}

	return E_OK;
}

// vim: tabstop=4
