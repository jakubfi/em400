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
#include "mjc400_timer.h"
#include "mjc400_regs.h"

// -----------------------------------------------------------------------
void mjc400_timer_interrupt(int i)
{
	RZ_5sb;
}

// -----------------------------------------------------------------------
int mjc400_timer_start()
{
	struct itimerval it_val;
	struct sigaction act;

	act.sa_handler = mjc400_timer_interrupt;
	act.sa_flags = 0;
	
	if (sigaction(SIGALRM, &act, NULL) != 0) {
		return 1;
	}

	it_val.it_value.tv_sec = 0;
	it_val.it_value.tv_usec = MJC400_TIMER * 1000;
	it_val.it_interval = it_val.it_value;

	if (setitimer(ITIMER_REAL, &it_val, NULL) == -1) {
		return 1;
	}

	return 0;
}
