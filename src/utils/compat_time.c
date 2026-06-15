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

#include "compat_time.h"

#ifdef _WIN32

#include <windows.h>
#include <stdint.h>

// Win10 1803+ high-resolution waitable timer flag.
#ifndef CREATE_WAITABLE_TIMER_HIGH_RESOLUTION
#define CREATE_WAITABLE_TIMER_HIGH_RESOLUTION 0x00000002
#endif

// -----------------------------------------------------------------------
static HANDLE open_timer(void)
{
	HANDLE h = CreateWaitableTimerExW(NULL, NULL, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);
	if (!h) {
		// Pre-1803 fallback: regular waitable timer (~1 ms with timeBeginPeriod(1))
		h = CreateWaitableTimerExW(NULL, NULL, 0, TIMER_ALL_ACCESS);
	}
	return h;
}

// -----------------------------------------------------------------------
void compat_sleep_until(const struct timespec *deadline)
{
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);

	int64_t rel_ns = ((int64_t)deadline->tv_sec  - (int64_t)now.tv_sec)  * 1000000000LL
	               + ((int64_t)deadline->tv_nsec - (int64_t)now.tv_nsec);

	if (rel_ns <= 0) return;

	// Reuse a per-thread timer handle across calls.
	static _Thread_local HANDLE htimer;
	if (!htimer) {
		htimer = open_timer();
		if (!htimer) return;
	}

	// SetWaitableTimer takes 100-ns units; negative value = relative interval.
	LARGE_INTEGER due;
	due.QuadPart = -(rel_ns / 100);
	if (due.QuadPart == 0) return;

	SetWaitableTimer(htimer, &due, 0, NULL, NULL, FALSE);
	WaitForSingleObject(htimer, INFINITE);
}

#endif // _WIN32

// vim: tabstop=4 shiftwidth=4 autoindent
